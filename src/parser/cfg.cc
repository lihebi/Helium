#include "cfg.h"
#include "ast_node.h"

/**
 * Get the exit node.
 * If a ordinary statement, it is itself;
 * If a branch, it is the last stmt of all the exit of the branch;
 * If a loop, it is the loop condition, the break;
 */
std::vector<ASTNode*> get_exit_nodes(ASTNode *ast_node) {
  std::vector<ASTNode*> ret;
  switch (ast_node->Kind()) {
  case ANK_Do:
  case ANK_While:
  case ANK_For:
    // loop
    break;
  case ANK_If:
    break;
  default:
    break;
  }
  return ret;
}

void CFG::traverse(CFGNode *node) {
  if (!node) return;
  ASTNode *ast_node = node->GetASTNode();
  switch (ast_node->Kind()) {
  case ANK_If: traverseIf(node); break;
  case ANK_Function: traverseFunction(node); break;
  case ANK_ElseIf: traverseElseIf(node); break;
  case ANK_Switch: traverseSwitch(node); break;
  case ANK_While: traverseWhile(node); break;
  case ANK_For: traverseFor(node); break;
  case ANK_Do: traverseDo(node); break;
  default: return;
  }
}

/**
 * 1. create CFGNode for all its children
 * 2.4 recursively traverse child
 * 2. Add edge to first child in each branch
 * 3. Add edge to connect if, elseif, elseif, else
 */
void CFG::traverseIf(CFGNode *node) {
  if (!node) return;
  If *if_node = dynamic_cast<If*>(node->GetASTNode());
  // If
  Then *then_node = if_node->GetThen();
  Else *else_node = if_node->GetElse();
  std::vector<ElseIf*> elseif_nodes = if_node->GetElseIfs();
  // then
  if (then_node) {
    CFGNode *child = GetCFGNode(then_node->GetFirstChild());
    if (child) {
      CreateEdge(node, child);
    }
  }
  CFGNode *current = node;
  for (ElseIf* else_if : elseif_nodes) {
    CFGNode *elseif_node = GetCFGNode(else_if);
    if (elseif_node) {
      CreateEdge(current, elseif_node);
      current = elseif_node;
      traverse(elseif_node);
    }
  }
  // else
  if (else_node) {
    CFGNode *child = GetCFGNode(else_node->GetFirstChild());
    if (child) {
      CreateEdge(current, child);
    }
  }
}


/**
 * 1. Create all the nodes
 * 2. connect all basic blocks
 * 2. For all the nodes, connect them with their children
 * 3. for all continue, connect to the condition
 * 4. For all break, connect to the next stmt
 * 3. For the structures, 
 */


/**
 * Construct CFG from AST
 */
CFG::CFG(AST *ast) : m_ast(ast) {
  assert(ast);
  // ASTNode *ast_root = ast->GetRoot();
  // CFGNode *cfg_node = new CFGNode(ast_root);
  std::vector<ASTNode*> astnodes = ast->GetNodes();
  for (ASTNode *astnode : astnodes) {
    CreateCFGNode(astnode);
  }
  // finding root
  CFGNode *root = GetCFGNode(ast->GetRoot());
  traverse(root);
  // creating edges
  for (CFGNode *cfgnode : m_nodes) {
    ASTNode *astnode = cfgnode->GetASTNode();
    if (dynamic_cast<Function*>(astnode)) {
      // function
      m_root = cfgnode;
    } else if (If *if_node = dynamic_cast<If*>(astnode)) {
    } else if (ElseIf *elseif_node = dynamic_cast<ElseIf*>(astnode)) {
      // Else If
      CFGNode *child = GetCFGNode(elseif_node->Child(0));
      if (child) {
        CreateEdge(cfgnode, child);
      }
    } else if (For *for_node = dynamic_cast<For*>(astnode)) {
      CFGNode *child = GetCFGNode(for_node->GetFirstChild());
      if (child) {
        CreateEdge(cfgnode, child);
      }
      // child = GetCFGNode(for_node->GetLastChild());
      // if (child) {
      //   CreateExitEdge(child, cfgnode);
      // }
      ASTNode *astchild = for_node->GetLastChild();
      std::vector<ASTNode*> exits = get_exit_nodes(astchild);
      for (ASTNode *exit_node : exits) {
        CFGNode *exit_cfg = GetCFGNode(exit_node);
        CreateEdge(exit_cfg, cfgnode);
      }
    }
  }
}


void CFG::CreateEdge(CFGNode *from, CFGNode *to) {
  if (from && to) {
    from->AddSuccessor(to);
    to->AddPredecessor(from);
  }
}

void CFG::CreateCFGNode(ASTNode *astnode) {
  if (!astnode) return;
  if (GetCFGNode(astnode)) return;
  // constructing
  switch (astnode->Kind()) {
  case ANK_Then:
  case ANK_Else:
  case ANK_Other:
    return;
  default: ;
  }
  // create
  CFGNode *cfgnode = new CFGNode(astnode);
  m_nodes.push_back(cfgnode);
  m_ast2cfg[astnode] = cfgnode;
}

CFGNode *CFG::GetCFGNode(ASTNode *astnode) {
  if (m_ast2cfg.count(astnode) == 1) {
    return m_ast2cfg[astnode];
  }
  return NULL;
}
