#include "segment.h"
#include "parser/resource.h"
#include "utils/log.h"
#include <iostream>
#include "generator.h"
#include "utils/utils.h"
#include "resolver/global_variable.h"
#include "helium_options.h"

CFGNode* Segment::m_poi = NULL;

Segment::Segment(ASTNode *astnode) {
  assert(astnode);
  CFG *cfg = Resource::Instance()->GetCFG(astnode->GetAST());
  CFGNode *cfgnode = cfg->ASTNodeToCFGNode(astnode);
  m_selection.insert(cfgnode);
  m_new.insert(cfgnode);
  m_head = cfgnode;
}


Segment::Segment(const Segment &q) {
  m_selection = q.m_selection;
  m_new = q.m_new;
  m_head = q.m_head;
}

Segment::Segment(CFGNode *cfgnode) {
  // TODO ensure this is not already in m_selection, otherwise the m_new will not be ideal
  m_selection.insert(cfgnode);
  m_new.insert(cfgnode);
  m_head = cfgnode;
}

void Segment::Merge(Segment *q) {
  std::set<CFGNode*> nodes = q->GetSelection();
  m_selection.insert(nodes.begin(), nodes.end());
}

void Segment::Add(CFGNode *node, bool inter) {
  assert(node);
  m_selection.insert(node);
  m_head = node;
  m_new.insert(node);
  if (inter) {
    m_callsites.insert(node);
  }
}


/**
 * Remove a node.
 * This node is typically marked as "bad".
 * This node is typically m_new;
 * But I will not change m_new to something else, because I need to do context search from there.
 */
void Segment::Remove(CFGNode *node) {
  if (m_callsites.count(node) == 1) {
    m_valid = false;
  } else if (m_selection.count(node) == 1) {
    m_selection.erase(node);
  }
  if (!m_poi || m_selection.count(m_poi)==0) m_valid = false;
}

void Segment::Remove(std::set<CFGNode*> nodes) {
  for (CFGNode *node : nodes) {
    if (m_callsites.count(node) == 1) {
      m_valid = false;
      return;
    }
    m_selection.erase(node);
  }
  if (!m_poi || m_selection.count(m_poi)==0) m_valid = false;
}

/**
 * Call PatchGrammar before resolving input and snippet, because the variable used in the condition might be a input variable
 */
void Segment::PatchGrammar() {
  // we want to resolve the switch statement here
  std::map<AST*, std::set<ASTNode*> > groups;
  for (CFGNode *cfgnode : m_selection) {
    ASTNode *astnode = cfgnode->GetASTNode();
    AST *ast = astnode->GetAST();
    groups[ast].insert(astnode);
  }

  // patch for all the asts
  for (auto m : groups) {
    AST *ast = m.first;
    std::set<ASTNode*> nodes = m.second;
    std::set<ASTNode*> newnodes = ast->PatchGrammar(nodes);
    CFG *cfg = Resource::Instance()->GetCFG(ast);
    for (ASTNode *diff : newnodes) {
      if (nodes.count(diff) == 0) {
        CFGNode *cfgnode = cfg->ASTNodeToCFGNode(diff);
        // inserting difference (new nodes) into selection
        m_selection.insert(cfgnode);
      }
    }
  }
}

bool Segment::RemoveNewBranch() {
  std::set<CFGNode*> to_remove;
  for (CFGNode *node : m_new) {
    // FIXME NOW may need to remove other nodes, e.g. if remove "IF", then need to remove "Then", "ElseIf", etc
    if (node->IsBranch()) {
      to_remove.insert(node);
    }
  }
  for (CFGNode *node : to_remove) {
    if (m_callsites.count(node) == 1) {
      m_valid = false;
      return false;
    }
    m_selection.erase(node);
    m_new.erase(node);
  }
  if (!m_poi || m_selection.count(m_poi)==0) m_valid = false;
  return !to_remove.empty();
}

bool Segment::ContainNode(CFGNode *node) {
  if (m_selection.count(node) == 1) return true;
  else return false;
}

/**
 * Visualize on CFG.
 * Only display the first CFG for now.
 */
// void Segment::Visualize(bool open) {
//   ASTNode *astnode = m_head->GetASTNode();
//   CFG *cfg = Resource::Instance()->GetCFG(astnode->GetAST());
//   // these nodes may not belong to this cfg
//   cfg->Visualize(m_selection, {m_new}, open);
// }


void Segment::ResolveInput() {
  helium_print_trace("Segment::ResolveInput");
  m_inputs.clear();
  AST *ast = m_head->GetASTNode()->GetAST();
  std::set<ASTNode*> first_astnodes;
  for (CFGNode *cfgnode : m_selection) {
    ASTNode *astnode = cfgnode->GetASTNode();
    if (astnode && ast->Contains(astnode)) {
      first_astnodes.insert(astnode);
    }
  }
  std::set<std::string> included;
  for (ASTNode *astnode : first_astnodes) {
    std::set<std::string> ids = astnode->GetVarIds();
    for (std::string id : ids) {
      if (id.empty()) continue;
      if (is_c_keyword(id)) continue;
      if (included.count(id) == 1) continue;
      SymbolTable *tbl = astnode->GetSymbolTable();
      SymbolTableValue *st_value = tbl->LookUp(id);
      if (st_value) {
        if (first_astnodes.count(st_value->GetNode()) == 0) {
          // (HEBI: input)
          m_inputs.push_back(new Variable(st_value->GetType(), id));
          included.insert(id);
          // m_inputs[st_value->GetName()] = st_value->GetType();
        }
      } else {
        // FIXME global variable should not be declared again
        Type *type = GlobalVariableRegistry::Instance()->LookUp(id);
        if (type) {
          // m_inputs[id] = type;
          // m_inputs_without_decl[id] = type;
          Variable *var = new Variable(type, id);
          var->SetGlobal();
          m_inputs.push_back(var);
          included.insert(id);
        }
      }
    }
  }
  if (HeliumOptions::Instance()->GetBool("print-input-variables")) {
    for (Variable *var : m_inputs) {
      std::string name = var->GetName();
      Type *type = var->GetType();
      if (!type) {
        std::cerr << "EE: when generating test suite, the type is NULL! Fatal error!" << "\n";
        exit(1);
      }
      std::cout << "Input Variables: " << "\t"
                << type->ToString() << ":" << type->GetRaw() << " " << name << "\n";
    }
  }

}


/**
 * Generate main, support, makefile, scripts
 */
void Segment::GenCode() {
  CodeGen generator;
  // generator.SetFirstAST(m_new->GetASTNode()->GetAST());

  for (CFGNode *cfgnode : m_selection) {
    if (cfgnode->GetASTNode()->Kind() == ANK_Switch) {
      std::set<CFGNode*> sucs = cfgnode->GetSuccessors();
      for (CFGNode *case_first : sucs) {
        // FIXME I can only check the first node here ...
        if (m_selection.count(case_first)==1) {
          CFGEdge *edge = cfgnode->GetCFG()->GetEdge(cfgnode, case_first);
          std::vector<ASTNode*> cases = edge->GetCases();
          for (ASTNode *c : cases) {
            generator.AddNode(c);
          }
        }
      }
    }
  }
  
  generator.SetFirstNode(m_head->GetASTNode());
  for (CFGNode *cfgnode : m_selection) {
    generator.AddNode(cfgnode->GetASTNode());
  }
  generator.SetInputs(m_inputs);


  // generator.Preprocess();

  
  m_main = generator.GetMain();
  m_support = generator.GetSupport();
  m_makefile = generator.GetMakefile();
}


std::string Segment::GetOpt() {
  std::string ret;
  if (m_main.find("getopt") != std::string::npos) {
    ret = m_main.substr(m_main.find("getopt"));
    std::vector<std::string> lines = utils::split(ret, '\n');
    assert(lines.size() > 0);
    ret = lines[0];
    assert(ret.find("\"") != std::string::npos);
    ret = ret.substr(ret.find("\"")+1);
    assert(ret.find("\"") != std::string::npos);
    ret = ret.substr(0, ret.find("\""));
    assert(ret.find("\"") == std::string::npos);
    // print out the ret
    std::cout << utils::CYAN << ret << utils::RESET << "\n";
  }
  return ret;
}
