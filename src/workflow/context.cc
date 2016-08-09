#include "context.h"
#include <iostream>
#include "utils/utils.h"

#include "parser/xml_doc_reader.h"
#include "parser/slice_reader.h"
#include "parser/ast_node.h"

#include "resolver/global_variable.h"
#include "resolver/snippet_db.h"

#include "builder.h"
#include "analyzer.h"
#include "tester.h"

#include "config/options.h"
#include "config/config.h"
#include "utils/log.h"
#include <gtest/gtest.h>

using namespace ast;
using namespace utils;

/********************************
 * Context
 *******************************/


/**
 * Create the very first context, with the segment itself.
 * The nodes for the context should be empty.
 * But the ast to node map should contain the POI already.
 * UPDATE: OK, I give up, keep the POI also in the context.
 */
Context::Context(Segment *seg) : m_seg(seg) {
  print_trace("Context::Context(Segment *seg)");
  // this is the beginning.
  // use the POI as the first node.
  SetFirstNode(seg->GetFirstNode());
  m_nodes = seg->GetPOI();
  for (ASTNode* node : m_nodes) {
    m_ast_to_node_m[node->GetAST()].insert(node);
  }

  m_search_time = 0;
}
// copy constructor
/**
 * Do I really need to manually copy these?
 * Maybe the default one already works good.
 */
Context::Context(const Context &rhs) {
  print_trace("Context::Context(const Context &rhs)");
  m_seg = rhs.m_seg;
  m_nodes = rhs.m_nodes;
  m_first = rhs.m_first;
  m_ast_to_node_m = rhs.m_ast_to_node_m;

  m_search_time = rhs.m_search_time;
}

Context::~Context() {
  if (m_builder) {
    delete m_builder;
  }
  freeTestSuite();
}

/********************************
 * Modifying
 *******************************/

/**
 * The first node denote which the most recent AST is.
 */
void Context::SetFirstNode(ast::ASTNode* node) {
  m_first = node;
}

bool Context::AddNode(ASTNode* node) {
  // insert here
  // but if test shows it should not be in, it will be removed
  if (m_nodes.count(node) == 1) return false;
  m_nodes.insert(node);
  m_ast_to_node_m[node->GetAST()].insert(node);
  // record context search time here
  m_search_time ++;
  return true;
}

void Context::RemoveNode(ASTNode *node) {
  // remove
  // FIXME assert exist
  m_nodes.erase(node);
  m_ast_to_node_m[node->GetAST()].erase(node);
}



/********************************
 * Debugging
 *******************************/

void Context::dump() {
  // separate nodes by their AST
  // std::map<AST*, std::set<ASTNode*> > asts;
  // for (ASTNode* node : m_nodes) {
  //   asts[node->GetAST()].insert(node);
  // }
  /**
   * Context does not maintain the nodes in POI.
   * Thus when dumping, need to take them into account.
   */
  // assert(m_seg);
  // for (ASTNode* node : m_seg->GetPOI()) {
  //   asts[node->GetAST()].insert(node);
  // }
  // for each AST, print out, or visualize
  // std::cout << "AST number: " << asts.size()  << "\n";
  static int count = 0;
  count ++;
  m_seg->UnDeclOutput();
  for (auto m : m_ast_to_node_m) {
    std::cout << "-----------"  << "\n";
    AST *ast = m.first;
    std::set<ASTNode*> nodes = m.second;
    std::string code = ast->GetCode(nodes);
    utils::print(code+"\n", utils::CK_Blue);
    // ast->VisualizeN(nodes, {}, std::to_string(count));
  }
  m_seg->DeclOutput();
}



/********************************
 * Resolving
 *******************************/

/**
 * TODO
 */
void Context::Resolve() {
  print_trace("Context::Resolve");
  // gather ASTs UPDATE already a member field: m_ast_to_node_m
  // resolve for each AST
  AST *first_ast = m_first->GetAST();
  for (auto m : m_ast_to_node_m) {
    AST *ast = m.first;
    std::set<ASTNode*> nodes = m.second;
    std::set<ASTNode*> complete;
    // if (ast != first_ast) {
    //   // this is inner function.
    //   // this one should not have input variables.
    //   // all the inputs should be resolved such that the input is self-sufficient
    //   // Complete to root to include the function prototype
    //   // FIXME can I modify the map here?
    //   // TODO complete def use
    //   complete = resolveDecl(ast, false);
    // } else {
    //   // this is the first AST
    //   // this one needs input variables
    //   complete = resolveDecl(ast, true);
    // }

    
    // std::set<ASTNode*> selection = m_ast_to_node_m[ast];
    // if (ast == first_ast) {
    //   // if it is first ast, need to add declaration and 
    //   selection = ast->CompleteGene(selection);
    //   selection = ast->RemoveRoot(selection);
    // } else {
    //   // need to add declaration
    //   // need to complete to root
    //   selection = ast->CompleteGeneToRoot(selection);
    // }
    // m_ast_to_node_m[ast] = selection;

    // TODO disabled def use analysis
    complete = resolveDecl(ast, ast == first_ast);
    m_ast_to_node_m[ast] = complete;
    // if (ast == first_ast) {
    //   complete = ast->RemoveRoot(complete);
    //   m_ast_to_node_m[ast] = complete;
    //   getUndefinedVariables(ast);
    // }
    
    // std::cout << complete.size()  << "\n";
    // ast->VisualizeN(complete, {});
    // getchar();
    resolveSnippet(ast);
  }
}

/**
 * This is for the first AST only
 * It resolve the variables to see if it can be resolved.
 * Then see if the Decl already included in selection
 * Otherwise, return as input variables
 *
 * HEBI this must be called after completion
 * HEBI Need to disable the first ast decl resolving
 * @return m_decls
 * DEPRECATED Dead code
 */
void Context::getUndefinedVariables(AST *ast) {
  print_trace("Context::getUndefinedVariables(AST *ast)");
  std::set<ASTNode*> sel = m_ast_to_node_m[ast];
  for (ASTNode *node : sel) {
    std::set<std::string> ids = node->GetVarIds();
    for (std::string id : ids) {
      if (id.empty()) continue;
      if (is_c_keyword(id)) continue;
      std::cout << id  << "\n";
      SymbolTable *tbl = node->GetSymbolTable();
      assert(tbl);
      SymbolTableValue *decl = tbl->LookUp(id);
      if (decl) {
        ASTNode *decl_node = decl->GetNode();
        if (decl_node) {
          if (sel.count(decl_node) == 1) continue;
          else {
            m_decls[ast][id] = decl->GetType();
          }
        }
      } else {
        // cannot found in symbol table
        std::cout << "..."  << "\n";
        Type* type = GlobalVariableRegistry::Instance()->LookUp(id);
        if (type) {
          std::cout << "found global variable: " << id  << "\n";
          m_decls[ast][id] = type;
        }
      }
    }
  }
  std::cout << "end"  << "\n";
}


/**
 * Resolve the declaration, resolve input variables
 * @return set of new selection
 * @return m_decl: this seems to be the only one used when generating main.c
 * @return m_input
 */
std::set<ASTNode*> Context::resolveDecl(AST *ast, bool first_ast_p) {
  print_trace("Context::resolveDecl");

  // ast->Visualize();
  std::set<ASTNode*> ret = m_ast_to_node_m[ast];
  // FIXME do I need to complete again after I recursively add the dependencies
  if (first_ast_p) {
    ret = ast->CompleteGene(ret);
    // return ret;
  } else {
    ret = ast->CompleteGeneToRoot(ret);
  }
  // ast->VisualizeN(ret, {});
  std::set<ASTNode*> worklist = ret; // m_ast_to_node_m[ast];
  std::set<ASTNode*> done;
  // from the decl node to the variable needed to be declared
  std::map<ASTNode*, std::set<std::string> > decl_input_m;
  // do not need input
  std::map<ASTNode*, std::set<std::string> > decl_m;
  // at the end, need to remove those node that already in m_gene
  while (!worklist.empty()) {
    ASTNode* node = *worklist.begin();
    worklist.erase(node);
    if (done.count(node) == 1) continue;
    done.insert(node);
    // find the var names
    // std::string code;
    // node->GetCode({}, code, true);
    // std::cout << code  << "\n";
    std::set<std::string> ids = node->GetVarIds();
    for (std::string id : ids) {
      if (id.empty()) continue;
      if (is_c_keyword(id)) continue;
      // DEBUG
      // std::cout <<id << " YYY at " << m_idx_m[node] << "\n";
      // std::cout << "resolving: " << id  << "\n";
      SymbolTable *tbl = node->GetSymbolTable();
      // tbl->dump();
      SymbolTableValue *decl = tbl->LookUp(id);
      ASTNode *def = node->LookUpDefinition(id);
      // FIXME I didn't use my new function resolve_type, which takes care of global variable and special variables like optarg
      // if (!decl) continue;
      if (decl) {
        // std::cout << "decl found for variable; " << id  << "\n";
        if (def) {
          // utils::print("found def: " + id + "\n", utils::CK_Cyan);
          // need this def, but not necessary to be a new node
          // and also possibly need the decl
          ret.insert(def);
          worklist.insert(def);
          // ast->VisualizeN({def}, {});
          // getchar();
          std::set<ASTNode*> morenodes = ast->CompleteGene(ret);
          ret.insert(morenodes.begin(), morenodes.end());
          worklist.insert(morenodes.begin(), morenodes.end());
          decl_m[decl->GetNode()].insert(id);
        } else {
          // may need decl, and need input
          decl_input_m[decl->GetNode()].insert(id);
        }
      } else {
        // cannot get decl from symbol table, might be a globle variable
        Type* type = GlobalVariableRegistry::Instance()->LookUp(id);
        if (type) {
          // add global variable
          if (m_globals.count(id) == 0) {
            m_globals[id] = type;
          }
        }
      }
    }
  }
  print_trace("Context::resolveDecl empty worklist");
  if (first_ast_p) {
    // FIXME Remove root for the outmost AST!
    ret = ast->RemoveRoot(ret);
  }
  // ret is the new complete, so just check the decorations not in ret
  for (auto it=decl_m.begin(), end=decl_m.end();it!=end;) {
    if (ret.count(it->first) == 1) {
      it = decl_m.erase(it);
    } else {
      ++it;
    }
  }
  for (auto it=decl_input_m.begin(), end=decl_input_m.end();it!=end;) {
    if (ret.count(it->first) == 1) {
      it = decl_input_m.erase(it);
    } else {
      ++it;
    }
  }
  // std::cout << decl_input_m.size()  << "\n";
  // std::cout << decl_m.size()  << "\n";
  // remove from the maps those that already in gene
  // for (ASTNode *n : done) {
  //   decl_input_m.erase(n);
  //   decl_m.erase(n);
  // }
  // std::cout << decl_input_m.size()  << "\n";
  // std::cout << decl_m.size()  << "\n";
  // store these two maps
  // m_decl_input_m = decl_input_m;
  // m_decl_m = decl_m;
  m_ast_to_deco_m[ast] = std::make_pair(decl_m, decl_input_m);
  for (auto m : decl_m) {
    for (std::string id : m.second) {
      SymbolTableValue *decl = m.first->GetSymbolTable()->LookUp(id);
      Type *t = decl->GetType();
      // assert(t);
      if (t) {
        m_decls[ast][id] = t;
      }
    }
  }
  for (auto m : decl_input_m) {
    for (std::string id : m.second) {
      SymbolTableValue *decl = m.first->GetSymbolTable()->LookUp(id);
      Type *t = decl->GetType();
      // assert(t);
      if (t) {
        m_decls[ast][id] = t;
        m_inputs[ast][id] = t;
      }
    }
  }
  print_trace("Context::resolveDecl end");
  // remove duplication of m_decls and m_inputs
  // the duplication is introduced, by the different DEFINITION in the code.
  // So, m_inputs should prominate it.
  return ret;
}

/**
 * TODO
 */
void Context::resolveSnippet(AST *ast) {
  print_trace("Context::resolveSnippet");
  std::set<std::string> all_ids;
  std::map<ASTNode*, std::set<std::string> > all_decls;
  // Since I changed the decl mechanism to m_decls, I need to change here
  
  // decl_deco decl_m = m_ast_to_deco_m[ast].first;
  // decl_deco decl_input_m = m_ast_to_deco_m[ast].second;
  // all_decls.insert(decl_input_m.begin(), decl_input_m.end());
  // all_decls.insert(decl_m.begin(), decl_m.end());
  // for (auto item : all_decls) {
  //   ASTNode *node = item.first;
  //   std::set<std::string> names = item.second;
  //   for (std::string name : names) {
  //     SymbolTableValue *value = node->GetSymbolTable()->LookUp(name);
  //     std::string type = value->GetType()->Raw();
  //     std::set<std::string> ids = extract_id_to_resolve(type);
  //     all_ids.insert(ids.begin(), ids.end());
  //   }
  // }

  /**
   * I want to resolve the type of the decls
   * And also the char array[MAX_LENGTH], see that macro?
   */
  for (auto m : m_decls[ast]) {
    std::string var = m.first;
    Type *t = m.second;
    std::string raw = t->Raw();
    // the type raw itself (does not contain dimension suffix)
    std::set<std::string> ids = extract_id_to_resolve(raw);
    all_ids.insert(ids.begin(), ids.end());
    // dimension suffix
    std::string dim = t->DimensionSuffix();
    ids = extract_id_to_resolve(dim);
    all_ids.insert(ids.begin(), ids.end());
  }
  // resolve the nodes selected by gene
  std::set<ASTNode*> nodes = m_ast_to_node_m[ast];
  for (ASTNode *n : nodes) {
    assert(n);
    std::set<std::string> ids = n->GetIdToResolve();
    all_ids.insert(ids.begin(), ids.end());
  }

  // I need to remove the function here!
  // otherwise the code snippet will get too much
  // For example, I have included a function in main.c, but it turns out to be here
  // even if I filter it out when adding to support.h, I still have all its dependencies in support.h!
  for (auto m : m_ast_to_node_m) {
    AST *ast = m.first;
    assert(ast);
    std::string func = ast->GetFunctionName();
    all_ids.erase(func);
  }

  // FIXME This output more than I want
  // e.g. Char GetOutputCode HELIUM_POI MAXPATHLEN Od_sizeof Od_strlen d fflush fileptr n printf stdout strcpy strlen tempname
  // some is in the string, e.g. HELIUM_POI
  // some is output variable, e.g. Od_sizeof
  // some is C reserved keywords, e.g. printf
  // std::cout << "ids to resolve snippets:"  << "\n";
  // for (auto id : all_ids) {
  //   std::cout << id  << " ";
  // }
  // std::cout << ""  << "\n";
  
  std::set<int> snippet_ids = SnippetDB::Instance()->LookUp(all_ids);
  // not sure if it should be here ..
  std::set<int> all_snippet_ids = SnippetDB::Instance()->GetAllDep(snippet_ids);
  all_snippet_ids = SnippetDB::Instance()->RemoveDup(all_snippet_ids);
  m_snippet_ids.insert(all_snippet_ids.begin(), all_snippet_ids.end());
  print_trace("Context::resolveSnippet end");
}

/********************************
 * Getting code
 *******************************/

/**
 * Get input variables used in this context.
 */
std::map<std::string, Type*> Context::GetInputVariables() {
  AST *first = m_first->GetAST();
  std::map<std::string, Type*> decls = m_decls[first];
  return decls;
}

std::string Context::getMain() {
  print_trace("Context::getMain()");
  std::string ret;
  ret += get_header();
  std::string main_func;
  std::string other_func;
  /**
   * Go through all the ASTs
   * The first AST should be placed in main function.
   * Other ASTs should just output.
   */
  AST *first_ast = m_first->GetAST();
  for (auto m : m_ast_to_node_m) {
    AST *ast = m.first;
    std::set<ASTNode*> nodes = m.second;
    if (ast == first_ast) {
      main_func += "int main() {\n";
      main_func += "int helium_size;\n"; // array size of heap
      // TODO need to call that function
      // ast->SetDecoDecl(m_ast_to_deco_m[ast].first);
      // ast->SetDecoDeclInput(m_ast_to_deco_m[ast].second);


      // Adding decls
      // This also adds input code!
      // m_inputs is not considered
      for (auto m :m_decls[ast]) {
        std::string var = m.first;
        Type *t = m.second;
        main_func += t->GetDeclCode(var);
        // FIXME didn't not use def use analysis result!
        main_func += t->GetInputCode(var);
      }

      if (Config::Instance()->GetBool("test-global-variable")) {
        for (auto m : m_globals) {
          std::string var = m.first;
          Type *t = m.second;
          // global only need input code
          // FIXME check the name collision for decls and globals
          // FIXME the order of input generation
          // main_func += t->GetDeclCode(var);
          main_func += t->GetInputCode(var);
        }
      }
      
      // FIXME TODO this is inside the main function. The main function is declared as int main(int argc, char *argv[])
      // No, because we will generate argc and argv, the main function actually is int main()
      // so the return statement should be a integer.
      // some code may just "return",
      // some may return some strange value
      // If I want to know the return value, I first needs to know its value.
      // So, lets modify all the return statements to be return 35, indicating an error
      // the return value in this can be restricted to a perdefined constant, to indicate such situation.
      std::string code = ast->GetCode(nodes);
      ast->ClearDecl();
      // modify the code, specifically change all return statement to return 35;
      code = replace_return_to_35(code);
      main_func += code;
      main_func += "return 0;\n";
      main_func += "};\n";
    } else {
      // other functions
      // ast->SetDecl(m_ast_to_deco_m[ast].second, m_ast_to_deco_m[ast].first);
      decl_deco merge;
      decl_deco first = m_ast_to_deco_m[ast].first;
      decl_deco second = m_ast_to_deco_m[ast].second;
      merge.insert(first.begin(), first.end());
      merge.insert(second.begin(), second.end());
      // ast->SetDecoDecl(m_ast_to_deco_m[ast].first);
      // ast->SetDecoDeclInput(m_ast_to_deco_m[ast].second);
      // For other function, only set the necessary decl.
      // Do not get input
      ast->SetDecoDecl(merge);
      std::string code = ast->GetCode(nodes);
      ast->ClearDecl();
      other_func += code;
      other_func += "\n";
    }
  }
  // FIXME delaration of other functions should be included in suport.
  ret += other_func;
  ret += main_func;
  return ret;
}


/**
 * FIXME dangerous! will clear the output decoration for all ASTs
 */
std::string Context::getSigMain() {
  print_trace("Context::getSigMain()");
  std::string ret;
  ret += get_header();
  std::string main_func;
  std::string other_func;
  /**
   * Go through all the ASTs
   * The first AST should be placed in main function.
   * Other ASTs should just output.
   */
  AST *first_ast = m_first->GetAST();
  for (auto m : m_ast_to_node_m) {
    AST *ast = m.first;
    std::set<ASTNode*> nodes = m.second;
    if (ast == first_ast) {
      main_func += "int main() {\n";
      ast->HideOutput();
      std::string code = ast->GetCode(nodes);
      ast->RestoreOutput();
      // modify the code, specifically change all return statement to return 35;
      code = replace_return_to_35(code);
      main_func += code;
      main_func += "return 0;";
      main_func += "};\n";
    } else {
      // other functions
      // ast->SetDecl(m_ast_to_deco_m[ast].second, m_ast_to_deco_m[ast].first);
      decl_deco merge;
      // decl_deco first = m_ast_to_deco_m[ast].first;
      // decl_deco second = m_ast_to_deco_m[ast].second;
      // merge.insert(first.begin(), first.end());
      // merge.insert(second.begin(), second.end());
      // ast->SetDecoDecl(merge);
      ast->HideOutput();
      std::string code = ast->GetCode(nodes);
      ast->RestoreOutput();
      other_func += code;
      other_func += "\n";
    }
  }
  // FIXME delaration of other functions should be included in suport.
  ret += other_func;
  ret += main_func;
  return ret;
}

std::string Context::getSupport() {
  std::string code = "";
  // head
  code += get_head();
  std::set<std::string> avail_headers = SystemResolver::Instance()->GetAvailableHeaders();
  std::set<std::string> used_headers = HeaderSorter::Instance()->GetUsedHeaders();
  for (std::string header : avail_headers) {
    if (used_headers.count(header) == 1) {
      code += "#include <" + header + ">\n";
    }
  }
  // code += SystemResolver::Instance()->GetHeaders();

  code += ""
    // setbit is DEFINE-d by Mac ...
    // <sys/param.h>
    // FIXME remove this header?
    "#ifdef setbit\n"
    "#undef setbit\n"
    "#endif\n";

  code += getSupportBody();
  // foot
  code += get_foot();
  return code;
}

std::string Context::getSupportBody() {
  std::string code;
  std::vector<int> sorted_snippet_ids = SnippetDB::Instance()->SortSnippets(m_snippet_ids);
  code += "\n/****** codes *****/\n";
  // snippets
  std::string code_func_decl;
  std::string code_variable;
  std::string code_func;
  /**
   * Avoid all the functions
   */
  std::set<std::string> avoid_funcs;
  for (auto m : m_ast_to_node_m) {
    avoid_funcs.insert(m.first->GetFunctionName());
  }
  // but, but, we should not avoid the function for the "first node", because it is in the main function
  avoid_funcs.erase(m_first->GetAST()->GetFunctionName());

  // DEBUG printing out avoid functions

  // for (auto &s : avoid_funcs) {
  //   std::cout << s  << "\n";
  // }

  // decl the avoid func here
  for (std::string s : avoid_funcs) {
    std::set<int> ids = SnippetDB::Instance()->LookUp(s, {SK_Function});
    // std::cout << ids.size()  << "\n";
    // std::cout << s  << "\n";
    // assert(ids.size() == 1);
    if (ids.size() == 0) {
      helium_log_warning("Function " + s + " lookup size: 0");
      continue;
    } else if (ids.size() > 1) {
      helium_log_warning("Function " + s + " lookup size: " + std::to_string(ids.size()));
    }
    int id = *ids.begin();
    std::string code = SnippetDB::Instance()->GetCode(id);
    std::string decl_code = ast::get_function_decl(code);
    code_func_decl +=
      "// Decl Code for " + s + "\n"
      + decl_code + "\n";
    // std::cout << "decl code for " << s << " is: " << decl_code  << "\n";
    // std::cout << "original code is " << code  << "\n";
  }

  
  // std::cout << sorted_snippet_ids.size()  << "\n";
  // FIXME if two snippets are exactly the same code, only include once
  // e.g. char tmpbuf[2048], target[2048], wd[2048];
  std::set<std::string> code_set;
  for (int id : sorted_snippet_ids) {
    SnippetMeta meta = SnippetDB::Instance()->GetMeta(id);
    if (meta.HasKind(SK_Function)) {
      std::string func = meta.GetKey();
      if (avoid_funcs.count(func) == 0) {
        std::string tmp_code = SnippetDB::Instance()->GetCode(id);
        if (code_set.count(tmp_code) == 0) {
          code_set.insert(tmp_code);
          code_func += "/* ID: " + std::to_string(id) + " " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
          code_func += tmp_code + "\n";
          code_func_decl += get_function_decl(SnippetDB::Instance()->GetCode(id))+"\n";
        }
      } else {
        // assert(false);
        // add only function decls
        // FIXME It seems to be empty string
        // std::cout << func << " avoid detected"  << "\n";
        // std::string decl = get_function_decl(SnippetDB::Instance()->GetCode(id) + "\n");
        // code_func_decl += decl;
      }
    } else if (meta.HasKind(SK_Variable)) {
      // for variable, put it AFTER function decl
      // this is because the variable may be a function pointer decl, and it may use the a funciton.
      // But of course it should be before the function definition itself, because it is
      std::string tmp_code = SnippetDB::Instance()->GetCode(id);
      if (code_set.count(tmp_code) == 0) {
        code_set.insert(tmp_code);
        code_variable += "/* ID: " + std::to_string(id) + " " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
        code_variable += tmp_code + '\n';
      }
    } else {
      // every other support code(structures) first
      std::string tmp_code = SnippetDB::Instance()->GetCode(id);
      if (code_set.count(tmp_code) == 0) {
        code_set.insert(tmp_code);
        code += "/* ID: " + std::to_string(id) + " " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
        code += tmp_code + '\n';
      }
    }
  }

  code += "\n// function declarations\n";
  code += code_func_decl;
  code += "\n// variables and function pointers\n";
  code += code_variable;
  code += "\n// functions\n";
  code += code_func;
  return code;
}


/**
 * How to create ~/github/helium-lib
 * - create the folder
 * - create a dummy project
 * - gnulib-tool -s --import [module names]
 * - modify Makefile.am and configure.ac
 * - aclocal && autoconf && automake --add-missing && ./configure && make
 * libgnu.a will be in lib/ folder

 * The modules to import [update when needed]:
 * - exclude
 * - progname
 */

std::string Context::getMakefile() {
  std::string makefile;
  std::string cc = Config::Instance()->GetString("cc");
  makefile += "CC:=" + cc + "\n";
  // TODO test if $CC is set correctly
  // makefile += "type $(CC) >/dev/null 2>&1 || { echo >&2 \"I require $(CC) but it's not installed.  Aborting.\"; exit 1; }\n";
  makefile += ".PHONY: all clean test\n";
  makefile = makefile + "a.out: main.c\n"
    + "\t$(CC) -g "
    // comment out because <unistd.h> will not include <optarg.h>
    // + "-std=c11 "
    + "main.c "
    + (Config::Instance()->GetBool("address-sanitizer") ? "-fsanitize=address " : "")
    // gnulib should not be used:
    // 1. Debian can install it in the system header, so no longer need to clone
    // 2. helium-lib already has those needed headers, if installed correctly by instruction
    // + "-I$(HOME)/github/gnulib/lib " // gnulib headers
    +
    (Config::Instance()->GetBool("gnulib") ?
     "-I$(HOME)/github/helium-lib " // config.h
     "-I$(HOME)/github/helium-lib/lib " // gnulib headers
     "-L$(HOME)/github/helium-lib/lib -lgnu " // gnulib library
     : "")
    + "-I/usr/include/x86_64-linux-gnu " // linux headers, stat.h
    + SystemResolver::Instance()->GetLibs() + "\n"
    + "clean:\n"
    + "\trm -rf *.out\n"
    + "test:\n"
    + "\tbash test.sh";
    
    return makefile;
}

TEST(SegmentTestCase, ExecTest) {
  std::string cmd = "/tmp/helium-test-tmp.4B0xG7/a.out";
  std::string input = utils::read_file("/tmp/helium-test-tmp.4B0xG7/input/2.txt");
  int status = 0;
  std::string output = utils::exec_in(cmd.c_str(), input.c_str(), &status, 10);
  std::cout << status  << "\n";
  std::cout << output  << "\n";
}

std::string run_script_filename = "run.sh";
std::string run_script =R"prefix(
#!/bin/bash
for input in input/*.txt; do
  echo \"===== $input =====\"
  ./a.out <$input
done
)prefix";


/**
 * Create builder and Compile
 * @return compile success or not
 * @side delete m_builder if it has value, fill in m_builder
 */
bool Context::compile() {
  if (m_builder) {
    delete m_builder;
    m_builder = NULL;
  }
  m_builder = new Builder();
  std::string code_main = getMain();
  std::string code_sup = getSupport();
  std::string code_make = getMakefile();

  /**
   * Signature Output
   */
  std::string code_sig_main = getSigMain();
  std::string code_sig_support = getSupportBody();
  
  m_builder->SetMain(code_main);
  m_builder->SetSupport(code_sup);
  m_builder->SetMakefile(code_make);
  m_builder->AddScript(run_script_filename, run_script);

  m_sig_dir = m_builder->GetDir() + "/sig";
  utils::create_folder(m_sig_dir);
  utils::write_file(m_sig_dir + "/sig_main.c", code_sig_main);
  utils::write_file(m_sig_dir + "/sig_support.h", code_sig_support);
  // to get the size of bug signature, just use =cloc xxx/sig=


  
  m_builder->Write();
  if (PrintOption::Instance()->Has(POK_CodeOutputLocation)) {
    std::cout << "Code output to "  << m_builder->GetDir() << "\n";
  }
  if (PrintOption::Instance()->Has(POK_Main)) {
    std::cout << code_main  << "\n";
  }
  m_builder->Compile();
  if (m_builder->Success()) {
    g_compile_success_no++;
    helium_dump("compile success\n");
    if (PrintOption::Instance()->Has(POK_CompileInfo)) {
      utils::print("compile success\n", utils::CK_Green);
    }
    if (PrintOption::Instance()->Has(POK_CompileInfoDot)) {
      utils::print(".", utils::CK_Green);
      std::cout << std::flush;
    }
    return true;
  } else {
    g_compile_error_no++;
    if (PrintOption::Instance()->Has(POK_CompileInfo)) {
      utils::print("compile error\n", utils::CK_Red);
    }
    helium_dump("compile error\n");
    if (PrintOption::Instance()->Has(POK_CompileInfoDot)) {
      utils::print(".", utils::CK_Red);
      std::cout << std::flush;
    }
    if (DebugOption::Instance()->Has(DOK_PauseCompileError)) {
      std::cout <<".. print enter to continue .."  << "\n";
      getchar();
    }

    // remove this first node
    RemoveNode(m_first);
    return false;
  }
  // TODO return good or not, to decide whether to keep the newly added statements
}

/**
 * Create test cases for Test.
 * @side m_test_suite get freed and cleared; filled with new test cases
 */
void Context::createTestCases() {
  print_trace("Context::createTestCases");
  // FIXME this function is too long
  if (Config::Instance()->GetBool("run-test") == false) {
    return;
  }
  int test_number = Config::Instance()->GetInt("test-number");
  // std::vector<std::vector<TestInput*> > test_suite(test_number);
  m_test_suite.clear();
  freeTestSuite();
  m_test_suite.resize(test_number);
  // decl_deco decl_input_m = m_ast_to_deco_m[m_first->GetAST()].second;

  /**
   * Testing!
   */
  print_trace("preparing inputs ...");
  AST *first_ast = m_first->GetAST();
  // FIXME decls or inputs?
  // InputMetrics metrics = m_inputs[first_ast];
  InputMetrics metrics = m_decls[first_ast];
  // metrics.insert(m_global.begin(), m_global.end());
    
  /**
   * Using TestInput
   */

  // when generating inputs, I need to monitor if the main file has the getopt staff
  // 	while( ( o = getopt( argc, argv, "achtvf:" ) ) != -1 ){
  // if yes, I need to get the spec
  // then when generating argc and argv, I need to be careful to cover each case
  // also, I need to mark the inputs as: argv_a, argv_c, argv_h
  // argv_a is binary
  // argv_f is a string!
  ArgCV argcv;
  std::string code_main = m_builder->GetMain();
  if (code_main.find("getopt") != std::string::npos) {
    std::string opt = code_main.substr(code_main.find("getopt"));
    std::vector<std::string> lines = utils::split(opt, '\n');
    assert(lines.size() > 0);
    opt = lines[0];
    assert(opt.find("\"") != std::string::npos);
    opt = opt.substr(opt.find("\"")+1);
    assert(opt.find("\"") != std::string::npos);
    opt = opt.substr(0, opt.find("\""));
    assert(opt.find("\"") == std::string::npos);
    // print out the opt
    utils::print(opt, utils::CK_Cyan);
    // set the opt
    argcv.SetOpt(opt);
  }

  // I should also capture the argc and argv variable used, but I can currently assume these variables here
  // Also, for regular argc and argv, I need also care about them, e.g. sizeof(argv) = argc, to avoid crashes.
    
  // I'm going to pre-generate argc and argv.
  // so that if later the metrics have that, I don't need to implement the match, just query
  std::vector<std::pair<TestInput*, TestInput*> > argcv_inputs = argcv.GetTestInputSpec(test_number);
  // used for freeing these inputs
  bool argc_used = false;
  bool argv_used = false;
    
  for (auto metric : metrics) {
    std::string var = metric.first;
    Type *type = metric.second;
    std::vector<TestInput*> inputs;
    if (var == "argc") {
      argc_used = true;
      for (auto p : argcv_inputs) {
        inputs.push_back(p.first);
      }
    } else if (var == "argv") {
      argv_used = true;
      for (auto p : argcv_inputs) {
        inputs.push_back(p.second);
      }
    } else {
      inputs = type->GetTestInputSpec(var, test_number);
    }
    assert((int)inputs.size() == test_number);
    for (int i=0;i<(int)inputs.size();i++) {
      m_test_suite[i].push_back(inputs[i]);
    }
  }

  if (Config::Instance()->GetBool("test-global-variable")) {
    // global inputs
    for (auto metric : m_globals) {
      std::string var = metric.first;
      Type *type = metric.second;
      std::vector<TestInput*> inputs;
      inputs = type->GetTestInputSpec(var, test_number);
      assert((int)inputs.size() == test_number);
      for (int i=0;i<(int)inputs.size();i++) {
        m_test_suite[i].push_back(inputs[i]);
      }
    }
  }

  // free when not used, to avoid memory leak
  if (!argc_used) {
    for (auto p : argcv_inputs) {
      delete p.first;
    }
  }
  if (!argv_used) {
    for (auto p : argcv_inputs) {
      delete p.second;
    }
  }
}

TestResult* Context::test() {
  print_trace("Context::test");
  // this is the other use place of test suite other than the execution of the executable itself
  // create the test result!
  // This will supply the input spec for the precondition and transfer function generation
  // The used method is ToString()
  TestResult *ret = new TestResult(m_test_suite);

  // std::string test_dir = utils::create_tmp_dir();
  utils::create_folder(m_builder->GetDir() + "/input");
  // if (m_test_suite.size() > 0 && PrintOption::Instance()->Has(POK_IOSpec)) {
  //   utils::print("TestinputMetrics:\n", CK_Blue);
  //   for (TestInput *in : m_test_suite[0]) {
  //     assert(in);
  //     utils::print(in->dump(), CK_Purple);
  //     // utils::print(in->GetRaw() + "\n", CK_Cyan);
  //   }
  // }
  // do the test
  for (int i=0;i<(int)m_test_suite.size();i++) {
    // std::string test_file = test_dir + "/test" + std::to_string(i) + ".txt";
    // utils::write_file(test_file, m_test_suite[i]);
    // std::string cmd = m_builder->GetExecutable() + "< " + test_file + " 2>/dev/null";
    // std::cout << cmd  << "\n";
    std::string cmd = m_builder->GetExecutable();
    int status;
    // FIXME some command cannot be controled by time out!
    // std::string output = utils::exec(cmd.c_str(), &status, 1);
    std::string input;
    std::string spec;
    for (TestInput *in : m_test_suite[i]) {
      if (in) {
        input += in->GetRaw() + "\n";
        spec += in->dump() + "\n";
        spec += in->ToString() + "\n\n";
      }
    }

    // DEBUG
    // std::cout << "============== Test Input ================"  << "\n";
    // std::cout << "------------------------------" << "\n";
    // std::cout << input  << "\n";
    // std::cout << "------------------------------"  << "\n";
    // std::cout << spec  << "\n";

    // write IO spec file
    utils::write_file(m_builder->GetDir() + "/input/" + std::to_string(i) + ".txt" + ".spec", spec);
      
    // std::string output = utils::exec_in(cmd.c_str(), test_suite[i].c_str(), &status, 10);
    // I'm also going to write the input file in the executable directory
    utils::write_file(m_builder->GetDir() + "/input/" + std::to_string(i) + ".txt", input);
    // FIXME this timeout should be configurable?
    // FIXME how to safely and consistently identify timeout or not?
    std::string output = utils::exec_in(cmd.c_str(), input.c_str(), &status, 0.3);
    if (status == 0) {
      if (PrintOption::Instance()->Has(POK_TestInfo)) {
        utils::print("test success\n", utils::CK_Green);
      }
      if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
        utils::print(".", utils::CK_Green);
      }
      ret->AddOutput(output, true);
    } else {
      if (PrintOption::Instance()->Has(POK_TestInfo)) {
        utils::print("test failure\n", utils::CK_Red);
      }
      if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
        utils::print(".", utils::CK_Red);
      }
      ret->AddOutput(output, false);
    }
  }
  if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
    std::cout << "\n";
  }
  return ret;
}

void Context::analyze(TestResult *test_result) {
  print_trace("Context::analyze");
  test_result->PrepareData();
  // HEBI Generating CSV file
  std::string csv = test_result->GenerateCSV();
  // This is The whole IO file
  // I also want to write the valid IO file
  std::string csv_file = m_builder->GetDir() + "/io.csv";
  utils::write_file(csv_file, csv);
  std::cout << "Output to: " << csv_file   << "\n";
  if (PrintOption::Instance()->Has(POK_CSV)) {
    std::string cmd = "column -s , -t " + csv_file;
    std::string output = utils::exec(cmd.c_str());
    std::cout << output  << "\n";
  }
  test_result->GetInvariants();
  test_result->GetPreconditions();
  test_result->GetTransferFunctions();
  
  Analyzer analyzer(csv_file, m_seg->GetConditions());
  // TODO NOW
  // TestSummary summary = analyzer.GetSummary();
  // if ((double)(summary.reach_poi_test_success + summary.reach_poi_test_success) / summary.total_test < 0.1) {
  //   // hard to trigger, go to simplify approach
  //   simplify();
  // }
  std::vector<std::string> invs = analyzer.GetInvariants();
  std::vector<std::string> pres = analyzer.GetPreConditions();
  std::vector<std::string> trans = analyzer.GetTransferFunctions();
  if (PrintOption::Instance()->Has(POK_AnalysisResult)) {
    std::cout << "== invariants"  << "\n";
    for (auto &s : invs) {
      std::cout << "\t" << s  << "\n";
    }
    std::cout << "== pre condtions"  << "\n";
    for (auto &s : pres) {
      std::cout << "\t" << s  << "\n";
    }
    std::cout << "== transfer functions ------"  << "\n";
    for (auto &s : trans) {
      std::cout << "\t" << s  << "\n";
    }

    // std::string cmd = "compare.py -f " + csv_file;
    // std::string inv = utils::exec(cmd.c_str());
    // std::cout << inv  << "\n";
  }


  // std::cout << "---- resolveQuery -----"  << "\n";
  m_query_resolved = resolveQuery(invs, pres, trans);
  if (m_query_resolved) {
    std::cout << "== Query resolved!"  << "\n";
    // output some information to use in paper
    std::cout << "\t sig dir: " << m_sig_dir  << "\n";
    std::cout << "\t search time: " << m_search_time  << "\n";
  }
  // std::cout << "------ end of query resolving -----"  << "\n";
}

void Context::freeTestSuite() {
  for (std::vector<TestInput*> &v : m_test_suite) {
    for (TestInput *in : v) {
      if (in) {
        delete in;
      }
    }
  }
  m_test_suite.clear();
}

/**
 * Test this context.
 * 1. get code and create builder. Write and compile.
 * 2. create test cases
 * 3. test and collect test result
 * 4. generate dynamic properties
 */
void Context::Test() {
  print_trace("Context::Test");
  std::cout << "============= Context::Test() ================="  << "\n";
  std::cout << "The size of this context: " << m_nodes.size()  << "\n";
  if (compile()) {
    createTestCases();
    TestResult *test_result = test();
    analyze(test_result);
    delete test_result;
  }
}


/**
 * Simplify out branches.
 * 1. examine if the latest context is a branch, loop condition.
 * 2. use symbolic execution to determine the input space to trigger that condition
 * 3. construct two contexts, with and without (needs to move the statement out of that branch)
 * 4. test the pre-condition for failure runs
 * 5. compare pre-conditions
 */
void Context::simplify() {
  // compare the different nodes of this context and last context
  // FIXME this is not very good, ideally we should only select nodes that dominates POI
  std::set<ASTNode*> last_nodes = m_last->GetNodes();
  std::set<ASTNode*> difference;
  for (ASTNode *node : m_nodes) {
    if (last_nodes.count(node) == 0) {
      difference.insert(node);
    }
  }
  // check the difference nodes
  ASTNode *problem_node;
  for (ASTNode *node : difference) {
    switch (node->Kind()) {
    case ANK_If:
    case ANK_ElseIf: {
    // case ANK_Do:
    // case ANK_For:
    // case ANK_While: {
      // XMLNode cond_node = node->GetCondition();
      // std::string cond = get_text(cond_node);
      problem_node = node;
      break;
    }
    default: ;
    }
    if (problem_node) break;
  }
  if (!problem_node) return;

  /**
   * Symbolic Execution
   */
  XMLNode cond_node = problem_node->GetCondition();
  std::string cond = get_text(cond_node);
  // Formula *formula = FormulaFactory::CreateFormula(cond);
  // TODO
  // formula->GetInputSpec();
  /**
   * Creating two contexts
   * Actually I don't really need to create two new context.
   * What I need to do is, get use "this" and "last" context to test.
   * For "this", use the input space got from symbolic execution.
   * For "last", no need special care.
   */
  // TODO Test them
  // Compare pre-conditions
  // decide whether to discard this branch condition
  // DONE!
}

void free_binary_formula(std::vector<BinaryFormula*> bfs) {
  for (BinaryFormula *bf : bfs) {
    delete bf;
  }
}


/**
 * I want to start from invs, try to use pres and trans to derive it.
 * Then I need to check the pres to see if those variables are entry point (argv, argc, optarg)
 */
bool Context::resolveQuery(std::vector<std::string> str_invs, std::vector<std::string> str_pres, std::vector<std::string> str_trans) {
  // Construct binary forumlas
  std::vector<BinaryFormula*> invs;
  std::vector<BinaryFormula*> pres;
  std::vector<BinaryFormula*> trans;
  // create BinaryFormula here. CAUTION need to free them
  for (std::string &s : str_invs) {
    invs.push_back(new BinaryFormula(s));
  }
  for (std::string &s : str_pres) {
    pres.push_back(new BinaryFormula(s));
  }
  for (std::string &s : str_trans) {
    trans.push_back(new BinaryFormula(s));
  }
  if (invs.empty() || trans.empty() || pres.empty()) return false;
  // how to identify the key invariant?
  // the constant invariants is used for derive
  // the relationship invariants are used as key
  // any relationship invariants is enough, for now
  BinaryFormula *key_inv = get_key_inv(invs);
  std::cout << "| selected the key invariants: " << key_inv->dump()  << "\n";
  
  // STEP Then, find the variables used in the key invariant
  // FIXME No, I need the whole header, e.g. strlen(fileptr). The sizeof(fileptr) is not useful at all.
  // find the related transfer functions
  // std::set<std::string> vars = key_inv->GetVars();
  std::vector<BinaryFormula*> related_trans; // = get_related_trans(trans, vars);
  for (BinaryFormula *bf : trans) {
    // std::string output_var = bf->GetLHSVar();
    // if (vars.count(output_var) == 1) {
    //   related_trans.push_back(bf);
    // }
    std::string item = bf->GetLHS();
    if (item == key_inv->GetLHS() || item == key_inv->GetRHS()) {
      related_trans.push_back(bf);
    }
  }

  // find the preconditions that define the input variables used in the transfer function
  std::set<std::string> related_input_items;
  for (BinaryFormula* bf : related_trans) {
    // FIXME y = x + z; I only want x
    related_input_items.insert(bf->GetRHS());
  }
  std::vector<BinaryFormula*> related_pres;
  for (BinaryFormula *bf : pres) {
    if (related_input_items.count(bf->GetLHS()) == 1 || related_input_items.count(bf->GetRHS()) == 1) {
      related_pres.push_back(bf);
    }
  }
  // use the preconditions and transfer funcitons to derive the invariants
  BinaryFormula* used_pre = derive_key_inv(related_pres, related_trans, key_inv);
  if (used_pre) {
    std::cout << "| Found the precondition that can derive to the invariant: " << used_pre->dump()  << "\n";
    // examine the variables in those preconditons, to see if they are entry points
    std::set<std::string> used_vars = used_pre->GetVars();
    // FIXME ugly
    free_binary_formula(pres);
    free_binary_formula(trans);
    free_binary_formula(invs);
    // for (BinaryFormula *bf : used_pres) {
    //   used_vars.insert(bf->GetLHSVar());
    //   used_vars.insert(bf->GetRHSVar());
    // }
    std::cout << "| The variables used: "  << "\n";
    for (std::string var : used_vars) {
      std::cout << "| " << var  << "\n";
      // this is argv:f!
      if (var.find(':') != std::string::npos) {
        var = var.substr(0, var.find(':'));
      }
      if (var != "argv" && var != "argc" && var != "optarg") {
        return false;
      }
      if (var == "argv" || var == "argc") {
        // the argv should come from main
        if (m_first->GetAST()->GetFunctionName() != "main") {
          return false;
        }
      }
    }
    utils::print("Context Searching Success!\n", utils::CK_Green);
    return true;
  } else {
    free_binary_formula(pres);
    free_binary_formula(trans);
    free_binary_formula(invs);
    return false;
  }
}
