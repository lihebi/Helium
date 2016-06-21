#include "seg.h"
#include "options.h"
#include <iostream>
#include "utils.h"
#include "snippet_db.h"
#include "xml_doc_reader.h"
#include "slice_reader.h"

#include "segment.h" // for get_header, etc
#include "resolver.h" // for SystemResolver
#include "builder.h"
#include "global_variable.h"
#include "config.h"
#include "analyzer.h"
#include <gtest/gtest.h>

using namespace ast;
using namespace utils;

/**
 * On xml node (pugixml), remove node, add a new node with tag name "tagname", and pure text value content
 * The node should NOT be the root node, otherwise assertion failure will occur.
 */
static void replace_xml_node(XMLNode node, std::string tagname, std::string content) {
  assert(node.parent());
  Node new_node = node.parent().insert_child_before(tagname.c_str(), node);
  new_node.append_child(pugi::node_pcdata).set_value(content.c_str());
  node.parent().remove_child(node);
}

/**
 * Replace all the return xxx; statement to return 35;
 */
static std::string replace_return_to_35(const std::string &code) {
  // TODO pattern matching or using paser?
  XMLDoc *doc = XMLDocReader::CreateDocFromString(code);
  XMLNode root = doc->document_element();
  XMLNodeList nodes = find_nodes(root, NK_Return);
  for (XMLNode node : nodes) {
    // not sure if I need to add 
    replace_xml_node(node, "return", "return 35;");
  }
  std::string ret = ast::get_text(root);
  delete doc;
  return ret;
}

/**
 * Resolve the type of a variable at an AST node.
 * Recall that Type* instance is created in the ASTNode, by means of Decl*.
 * This function will resolve:
 *   1. symbol table in AST (procedure level)
 *   2. Global Variables
 *   3. Special Variables, e.g. optarg
 */
NewType* resolve_type(std::string var, ASTNode *node) {
  print_trace("resolve_type(std::string var, ASTNode *node)");
  SymbolTable *tbl = node->GetSymbolTable();
  SymbolTableValue* value = tbl->LookUp(var);
  if (value) {
    return value->GetType();
  } else {
    NewType* type = GlobalVariableRegistry::Instance()->LookUp(var);
    if (type) return type;
    else {
      // special
      std::cerr << var  << "\n";
      assert(false);
    }
  }
}

Seg::Seg(ast::XMLNode xmlnode) {
  print_trace("Seg::Seg()");
  XMLNode function_node = get_function_node(xmlnode);
  assert(function_node);
  std::string func_name = function_get_name(function_node);
  // AST *ast = getAST(function_node);
  AST *ast = new AST(function_node);
  if (SimpleSlice::Instance()->IsValid()) {
    ast->SetSlice();
    // DEBUG
    // ast->VisualizeSlice();
    // getchar();
  }

  // dumping the ast
  // ast->Visualize();

  m_func_to_ast_m[func_name] = ast;
  m_asts.push_back(ast);
  // POI
  // FIXME Not sure if I should use GetEnclosingNodeByXMLNode
  ASTNode *astnode = ast->GetNodeByXMLNode(xmlnode);
  m_nodes.insert(astnode);


  // POI output statement decoration to AST

  assert(m_nodes.size() > 0);
  std::set<std::string> ids;
  for (ASTNode *poi_node : m_nodes) {
    std::set<std::string> _ids = poi_node->GetVarIds();
    ids.insert(_ids.begin(),_ids.end());
  }
  ASTNode *poi_node = *m_nodes.begin();
  for (std::string id : ids) {
    // std::cout << id  << "\n";
    if (id.empty()) continue;
    if (is_c_keyword(id)) continue;
    NewType *type = resolve_type(id, *m_nodes.begin());
    if (type) {
      m_output_vars[poi_node].push_back({type, id});
    }
  }
  
  // std::cout << "getting POI output vars"  << "\n";
  // for (ASTNode *poi_node : m_nodes) {
  //   std::set<std::string> ids = poi_node->GetVarIds();
  //   for (std::string id : ids) {
  //     // std::cout << id  << "\n";
  //     if (id.empty()) continue;
  //     if (is_c_keyword(id)) continue;
  //     SymbolTable *tbl = poi_node->GetSymbolTable();
  //     SymbolTableValue *decl = tbl->LookUp(id);
  //     if (decl) {
  //       m_deco[poi_node].insert(id);
  //     } else {
  //       // will need to query global variables
  //       // will need to query system types, like optarg
  //     }
  //   }
  // }
  // FIXME
  // assert(!m_deco.empty());
  // std::cout << "deco output:"  << "\n";
  // for (auto m : m_deco) {
  //   for (std::string s : m.second) {
  //     std::cout << s  << "\n";
  //   }
  // }
  // ast->SetDecoOutput(m_deco);
  ast->SetOutput(m_output_vars);

  /**
   * Debugging: print out output variables
   */
  // std::cout << "output variables:"  << "\n";
  if (PrintOption::Instance()->Has(POK_IOSpec)) {
    utils::print("Output Variables:\n", CK_Blue);
    for (auto m : m_output_vars) {
      for (NewVariable var : m.second) {
        // std::cout << var.GetType()->ToString()  << " " << var.GetName() << "\n";
        utils::print(var.GetType()->ToString() + "\n", CK_Blue);
        utils::print(var.GetType()->GetOutputCode(var.GetName()) + "\n", CK_Purple);
      }
    }
  }
  
  // Initial context
  // m_ctxs.push_back(new Ctx(this));
}
Seg::~Seg() {
  for (ast::AST *ast : m_asts) {
    delete ast;
  }
  for (ast::XMLDoc* doc : m_docs) {
    delete doc;
  }
  for (Ctx *ctx : m_ctxs) {
    delete ctx;
  }
}

/**
 * Get the next context.
 1. get the newest context
 2. get the first ASTNode
 3. get previous leaf node
 4. if interprocedure, query call graph, create a new AST and move from there

 @return true if need to continue. false to stop.
 */
bool Seg::NextContext() {
  print_trace("Seg::NextContext()");
  Ctx *ctx;
  if (m_ctxs.empty()) {
    ctx = new Ctx(this);
    m_ctxs.push_back(ctx);
    ctx->Resolve();
    ctx->Test();
    return true;
  }
  Ctx *last = m_ctxs.back();
  ctx = new Ctx(*last);
  m_ctxs.push_back(ctx);
  ASTNode *first_node = ctx->GetFirstNode();
  assert(first_node);
  assert(first_node->GetAST());
  // ASTNode *leaf = first_node->GetAST()->GetPreviousLeafNode(first_node);
  ASTNode *leaf = first_node->GetAST()->GetPreviousLeafNodeInSlice(first_node);
  /**
   * Note: the leaf should not be a declaration of a variable, which will be included if the variable is used.
   * It will not contribute to the property, thus no need to tes.
   */
  if (leaf) {
    // leaf is found
    std::string code;
    // leaf->GetCode({}, code, true);
    // utils::print(code, utils::CK_Blue);
    ctx->SetFirstNode(leaf);
    ctx->AddNode(leaf);
    // printing the new node
    // std::cout << leaf->dump()  << "\n";
    utils::print(leaf->dump() + "\n", utils::CK_Purple);
  } else {
    // std::cerr << "Inter procedure"  << "\n";
    AST *ast = first_node->GetAST();
    utils::print("Inter procedure: " + ast->GetFunctionName() + "\n", utils::CK_Cyan);
    if (ast->GetFunctionName() == "main") {
      utils::print("reach beginning of main. Stop.\n", utils::CK_Green);
      return false;
    }
    // FIXME wait, where did I free the doc?
    // XMLDoc *doc = createCallerDoc(ast);
    // // reach the function def
    // // query the callgraph, find call-sites, try one callsite.
    // // from that, form the new context
    // AST *newast = ASTFactory::CreateASTFromDoc(doc);
    XMLNode func_node = getCallerNode(ast);
    AST *newast = new AST(func_node);
    if (SimpleSlice::Instance()->IsValid()) {
      newast->SetSlice();
      // DEBUG
      // std::cout << newast->GetFilename() << "\n";
      // newast->VisualizeSlice();
      // getchar();
    }

    // dumping the ast
    // newast->Visualize();
    
    assert(newast);
    m_asts.push_back(newast);
    m_func_to_ast_m[newast->GetFunctionName()] = newast;
    // get callsite node
    // Node callsite_xmlnode = find_callsite(*doc, ast->GetFunctionName());
    // ASTNode *callsite = newast->GetEnclosingNodeByXMLNode(callsite_xmlnode);
    
    // if there're two functions in the project with the same name, I really have no idea which one to use.
    // Wait, I may have some clues by the filename and whether the callsite is in, but it is hard to implement, and not good solution.
    // The callsite may not be found if I choose the wrong one
    ASTNode *callsite = newast->GetCallSite(ast->GetFunctionName());
    assert(callsite);
    assert(callsite->GetAST());
    ctx->SetFirstNode(callsite);
    ctx->AddNode(callsite);
  }
  ctx->Resolve();
  // ctx->dump();
  ctx->Test();
  if (ctx->IsResolved()) {
    // query resolved.
    return false; // stop increasing
  } else {
    return true;
  }
  return true;
}

ast::XMLDoc* Seg::createCallerDoc(AST *ast) {
  print_trace("Seg::createCallerDoc()");
  std::string func = ast->GetFunctionName();
  std::string caller_func = SnippetDB::Instance()->QueryCaller(func);
  // std::cout << "caller function: " << caller_func  << "\n";
  // if the function AST is already created, this indicates a recursive function call.
  // TODO for now. Just assert
  assert(m_func_to_ast_m.count(caller_func) == 0 && "Recursive function call");
  std::set<int> caller_ids = SnippetDB::Instance()->LookUp(caller_func, {SK_Function});
  assert(!caller_ids.empty());
  int caller_id = *caller_ids.begin();
  // FIXME I should not create the doc here, by the code of the function only.
  // I want to keep the line numbers the same as orignal one
  // So, I may need to find the file (by the filename), and then get the function from that doc
  std::string func_code = SnippetDB::Instance()->GetCode(caller_id);
  std::string filename = SnippetDB::Instance()->GetMeta(caller_id).filename;
  // FIXME the filename is what in the snippet db, not the true filename that was passed by the command line
  // but they should be the same
  XMLDoc *doc = XMLDocReader::CreateDocFromString(func_code, filename);
  m_docs.push_back(doc);
  return doc;
}

/**
 * This will create a doc! The doc will be added into m_docs, and freed when the segment deconstruct
 */
ast::XMLNode Seg::getCallerNode(AST *ast) {
  print_trace("Seg::createCallerDoc()");
  std::string func = ast->GetFunctionName();
  std::string caller_func = SnippetDB::Instance()->QueryCaller(func);
  // std::cout << "caller function: " << caller_func  << "\n";
  // if the function AST is already created, this indicates a recursive function call.
  // TODO for now. Just assert
  assert(m_func_to_ast_m.count(caller_func) == 0 && "Recursive function call");
  std::set<int> caller_ids = SnippetDB::Instance()->LookUp(caller_func, {SK_Function});
  assert(!caller_ids.empty());
  int caller_id = *caller_ids.begin();
  // FIXME I should not create the doc here, by the code of the function only.
  // I want to keep the line numbers the same as orignal one
  // So, I may need to find the file (by the filename), and then get the function from that doc
  std::string func_code = SnippetDB::Instance()->GetCode(caller_id);
  std::string filename = SnippetDB::Instance()->GetMeta(caller_id).filename;
  // FIXME the filename is what in the snippet db, not the true filename that was passed by the command line
  // but they should be the same
  // FIXME this is not cached! Performance issue! See XMLDocReader doc for detail
  XMLDoc *doc = XMLDocReader::CreateDocFromFile(filename);
  XMLNodeList funcs = ast::find_nodes(doc->document_element(), NK_Function);
  m_docs.push_back(doc);
  for (XMLNode func : funcs) {
    if (function_get_name(func) == caller_func) {
      return func;
    }
  }
  assert(false);
}


/**
 * Create AST based on function node
 */
// AST* Seg::getAST(XMLNode function_node) {
//   assert(function_node && kind(function_node) == NK_Function);
//   if (m_asts_m.count(function_node) == 0) {
//     m_asts_m[function_node] = new AST(function_node);
//   }
//   return m_asts_m[function_node];
// }


/********************************
 * Context
 *******************************/


/**
 * Create the very first context, with the segment itself.
 * The nodes for the context should be empty.
 * But the ast to node map should contain the POI already.
 * UPDATE: OK, I give up, keep the POI also in the context.
 */
Ctx::Ctx(Seg *seg) : m_seg(seg) {
  print_trace("Ctx::Ctx(Seg *seg)");
  // this is the beginning.
  // use the POI as the first node.
  SetFirstNode(seg->GetFirstNode());
  m_nodes = seg->GetPOI();
  for (ASTNode* node : m_nodes) {
    m_ast_to_node_m[node->GetAST()].insert(node);
  }
}
// copy constructor
/**
 * Do I really need to manually copy these?
 * Maybe the default one already works good.
 */
Ctx::Ctx(const Ctx &rhs) {
  print_trace("Ctx::Ctx(const Ctx &rhs)");
  m_seg = rhs.m_seg;
  m_nodes = rhs.m_nodes;
  m_first = rhs.m_first;
  m_ast_to_node_m = rhs.m_ast_to_node_m;
}

/********************************
 * Modifying
 *******************************/

/**
 * The first node denote which the most recent AST is.
 */
void Ctx::SetFirstNode(ast::ASTNode* node) {
  m_first = node;
}

void Ctx::AddNode(ASTNode* node) {
  // insert here
  // but if test shows it should not be in, it will be removed
  m_nodes.insert(node);
  m_ast_to_node_m[node->GetAST()].insert(node);
}

void Ctx::RemoveNode(ASTNode *node) {
  // remove
  // FIXME assert exist
  m_nodes.erase(node);
  m_ast_to_node_m[node->GetAST()].erase(node);
}



/********************************
 * Debugging
 *******************************/

void Ctx::dump() {
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
  for (auto m : m_ast_to_node_m) {
    std::cout << "-----------"  << "\n";
    AST *ast = m.first;
    std::set<ASTNode*> nodes = m.second;
    std::string code = ast->GetCode(nodes);
    utils::print(code+"\n", utils::CK_Blue);
    // ast->VisualizeN(nodes, {}, std::to_string(count));
  }
}



/********************************
 * Resolving
 *******************************/

/**
 * TODO
 */
void Ctx::Resolve() {
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
 */
void Ctx::getUndefinedVariables(AST *ast) {
  print_trace("Ctx::getUndefinedVariables(AST *ast)");
  std::set<ASTNode*> sel = m_ast_to_node_m[ast];
  for (ASTNode *node : sel) {
    std::set<std::string> ids = node->GetVarIds();
    for (std::string id : ids) {
      if (id.empty()) continue;
      if (is_c_keyword(id)) continue;
      SymbolTable *tbl = node->GetSymbolTable();
      assert(tbl);
      SymbolTableValue *decl = tbl->LookUp(id);
      if (!decl) continue;
      ASTNode *decl_node = decl->GetNode();
      if (decl_node) {
        if (sel.count(decl_node) == 1) continue;
        else {
          m_decls[ast][id] = decl->GetType();
        }
      } else {
        NewType* type = GlobalVariableRegistry::Instance()->LookUp(id);
        if (type) {
          m_decls[ast][id] = type;
        }
      }
    }
  }
  std::cout << "end"  << "\n";
}


std::set<ASTNode*> Ctx::resolveDecl(AST *ast, bool first_ast_p) {
  print_trace("Ctx::resolveDecl");
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
      // std::cout <<id << " YYY at " << m_idx_m[node] << "\n";
      // std::cout << id  << "\n";
      SymbolTable *tbl = node->GetSymbolTable();
      // tbl->dump();
      SymbolTableValue *decl = tbl->LookUp(id);
      ASTNode *def = node->LookUpDefinition(id);
      if (!decl) continue;
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
    }
  }
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
      NewType *t = decl->GetType();
      assert(t);
      m_decls[ast][id] = t;
    }
  }
  for (auto m : decl_input_m) {
    for (std::string id : m.second) {
      SymbolTableValue *decl = m.first->GetSymbolTable()->LookUp(id);
      NewType *t = decl->GetType();
      assert(t);
      m_decls[ast][id] = t;
      m_inputs[ast][id] = t;
    }
  }
  // remove duplication of m_decls and m_inputs
  // the duplication is introduced, by the different DEFINITION in the code.
  // So, m_inputs should prominate it.
  return ret;
}

/**
 * TODO
 */
void Ctx::resolveSnippet(AST *ast) {
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
    NewType *t = m.second;
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
    std::set<std::string> ids = n->GetIdToResolve();
    all_ids.insert(ids.begin(), ids.end());
  }

  // I need to remove the function here!
  // otherwise the code snippet will get too much
  // For example, I have included a function in main.c, but it turns out to be here
  // even if I filter it out when adding to support.h, I still have all its dependencies in support.h!
  for (auto m : m_ast_to_node_m) {
    AST *ast = m.first;
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
}

/********************************
 * Getting code
 *******************************/

std::string Ctx::getMain() {
  print_trace("Ctx::getMain()");
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


      for (auto m :m_decls[ast]) {
        std::string var = m.first;
        NewType *t = m.second;
        main_func += t->GetDeclCode(var);
        // FIXME didn't not use def use analysis result!
        main_func += t->GetInputCode(var);
      }
      if (PrintOption::Instance()->Has(POK_IOSpec)) {
        utils::print("Input Metrics:\n", utils::CK_Blue);
        for (auto m :m_decls[ast]) {
          std::string var = m.first;
          NewType *t = m.second;
          utils::print(t->ToString() + "\n", utils::CK_Blue);
          utils::print(t->GetInputCode(var) + "\n", utils::CK_Purple);
          utils::print("-------\n", utils::CK_Blue);
          TestInput *input = t->GetTestInputSpec(var);
          // utils::print(t->GetTestInput() + "\n", utils::CK_Purple);
          utils::print(input->GetRaw() + "\n", utils::CK_Purple);
          delete input;
        }
      }
      // for (auto m : m_inputs[ast]) {
      //   std::string var = m.first;
      //   NewType *t = m.second;
      //   main_func += t->GetInputCode(var);
      //   std::cout << t->ToString()  << "\n";
      // }

      // print out the deco, to see if the same variable appear in both "first" and "second"
      // decl_deco deco = m_ast_to_deco_m[ast].first;
      // std::cout << "first"  << "\n";
      // for (auto m : deco) {
      //   for (std::string s : m.second) {
      //     std::cout << s  << "\n";
      //   }
      // }
      // deco = m_ast_to_deco_m[ast].second;
      // std::cout << "second"  << "\n";
      // for (auto m : deco) {
      //   for (std::string s : m.second) {
      //     std::cout << s  << "\n";
      //   }
      // }

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
      main_func += "return 0;";
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
std::string Ctx::getSupport() {
  std::vector<int> sorted_snippet_ids = SnippetDB::Instance()->SortSnippets(m_snippet_ids);
  std::string code = "";
  // head
  code += get_head();
  code += SystemResolver::Instance()->GetHeaders();
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
    assert(ids.size() == 1);
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
  for (int id : sorted_snippet_ids) {
    SnippetMeta meta = SnippetDB::Instance()->GetMeta(id);
    if (meta.HasKind(SK_Function)) {
      std::string func = meta.GetKey();
      if (avoid_funcs.count(func) == 0) {
        code_func += "/* ID: " + std::to_string(id) + " " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
        code_func += SnippetDB::Instance()->GetCode(id) + '\n';
        code_func_decl += get_function_decl(SnippetDB::Instance()->GetCode(id))+"\n";
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
      code_variable += "/* ID: " + std::to_string(id) + " " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
      code_variable += SnippetDB::Instance()->GetCode(id) + '\n';
    } else {
      // every other support code(structures) first
      code += "/* ID: " + std::to_string(id) + " " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
      code += SnippetDB::Instance()->GetCode(id) + '\n';
    }
  }

  code += "\n// function declarations\n";
  code += code_func_decl;
  code += "\n// variables and function pointers\n";
  code += code_variable;
  code += "\n// functions\n";
  code += code_func;
  // foot
  code += get_foot();
  return code;
}
std::string Ctx::getMakefile() {
  std::string makefile;
  makefile += ".PHONY: all clean test\n";
  makefile = makefile + "a.out: main.c\n"
    + "\tcc -g -std=c11 main.c " + SystemResolver::Instance()->GetLibs() + "\n"
    + "clean:\n"
    + "\trm -rf *.out\n"
    + "test:\n"
    + "\tbash test.sh";
    
    return makefile;
}

TEST(SegTestCase, ExecTest) {
  std::string cmd = "/tmp/helium-test-tmp.4B0xG7/a.out";
  std::string input = utils::read_file("/tmp/helium-test-tmp.4B0xG7/input/2.txt");
  int status = 0;
  std::string output = utils::exec_in(cmd.c_str(), input.c_str(), &status, 10);
  std::cout << status  << "\n";
  std::cout << output  << "\n";
}


void Ctx::Test() {
  std::cout << "============= Ctx::Test() ================="  << "\n";
  Builder builder;
  std::string code_main = getMain();
  std::string code_sup = getSupport();
  std::string code_make = getMakefile();
  builder.SetMain(code_main);
  builder.SetSupport(code_sup);
  builder.SetMakefile(code_make);
  builder.Write();
  if (PrintOption::Instance()->Has(POK_CodeOutputLocation)) {
    std::cout << "Code output to "  << builder.GetDir() << "\n";
  }
  if (PrintOption::Instance()->Has(POK_Main)) {
    std::cout << code_main  << "\n";
  }
  builder.Compile();
  if (builder.Success()) {
    g_compile_success_no++;
    if (PrintOption::Instance()->Has(POK_CompileInfo)) {
      utils::print("compile success\n", utils::CK_Green);
    }
    if (PrintOption::Instance()->Has(POK_CompileInfoDot)) {
      utils::print(".", utils::CK_Green);
      std::cout << std::flush;
    }

    // decl_deco decl_input_m = m_ast_to_deco_m[m_first->GetAST()].second;

    int test_number = Config::Instance()->GetInt("test-number");
    /**
     * Testing!
     */
    AST *first_ast = m_first->GetAST();
    InputMetrics metrics = m_inputs[first_ast];
    // std::vector<std::string> test_suites(TEST_NUMBER);
    // for (auto metric : metrics) {
    //   std::string var = metric.first;
    //   NewType *type = metric.second;
    //   std::vector<std::string> inputs = type->GetTestInput(TEST_NUMBER);
    //   assert(inputs.size() == TEST_NUMBER);
    //   for (int i=0;i<TEST_NUMBER;i++) {
    //     test_suites[i] += " " + inputs[i];
    //   }
    // }

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
    
    std::vector<std::vector<TestInput*> > test_suite(test_number);
    for (auto metric : metrics) {
      std::string var = metric.first;
      NewType *type = metric.second;
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
        test_suite[i].push_back(inputs[i]);
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

    // this is the other use place of test suite other than the execution of the executable itself
    // create the test result!
    // This will supply the input spec for the precondition and transfer function generation
    // The used method is ToString()
    NewTestResult test_result(test_suite);


    // std::string test_dir = utils::create_tmp_dir();
    utils::create_folder(builder.GetDir() + "/input");
    for (int i=0;i<test_number;i++) {
      // std::string test_file = test_dir + "/test" + std::to_string(i) + ".txt";
      // utils::write_file(test_file, test_suite[i]);
      // std::string cmd = builder.GetExecutable() + "< " + test_file + " 2>/dev/null";
      // std::cout << cmd  << "\n";
      std::string cmd = builder.GetExecutable();
      int status;
      // FIXME some command cannot be controled by time out!
      // std::string output = utils::exec(cmd.c_str(), &status, 1);
      std::string input;
      for (TestInput *in : test_suite[i]) {
        input += in->GetRaw() + " ";
      }

      if (PrintOption::Instance()->Has(POK_IOSpec)) {
        utils::print("TestinputMetrics:\n", CK_Blue);
        for (TestInput *in : test_suite[i]) {
          utils::print(in->dump() + "\n", CK_Purple);
        }
      }
      // std::string output = utils::exec_in(cmd.c_str(), test_suite[i].c_str(), &status, 10);
      // I'm also going to write the input file in the executable directory
      utils::write_file(builder.GetDir() + "/input/" + std::to_string(i) + ".txt", input);
      std::string output = utils::exec_in(cmd.c_str(), input.c_str(), &status, 10);
      if (status == 0) {
        if (PrintOption::Instance()->Has(POK_TestInfo)) {
          utils::print("test success\n", utils::CK_Green);
        }
        if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
          utils::print(".", utils::CK_Green);
        }
        test_result.AddOutput(output, true);
      } else {
        if (PrintOption::Instance()->Has(POK_TestInfo)) {
          utils::print("test failure\n", utils::CK_Red);
        }
        if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
          utils::print(".", utils::CK_Red);
        }
        test_result.AddOutput(output, false);
      }
      
      // std::cout << "output:"  << "\n";
      // std::cout << output  << "\n";
    }
    if (PrintOption::Instance()->Has(POK_TestInfoDot)) {
      std::cout << "\n";
    }

    test_result.PrepareData();
    // std::string i_csv = test_result.GenerateCSV("I", "S");
    // std::string o_csv = test_result.GenerateCSV("O", "S");
    // HEBI Generating CSV file
    std::string csv = test_result.GenerateCSV("IO", "SF");
    // std::cout << "icsv"  << "\n";
    // std::cout << i_csv  << "\n";
    // std::cout << "ocsv"  << "\n";
    // std::cout << o_csv  << "\n";
    // std::cout << "csv"  << "\n";
    // std::cout << csv  << "\n";
    /**
     * Save to file, and output file name.
     */
    // std::string tmp_dir = utils::create_tmp_dir();
    // utils::write_file(tmp_dir + "/i.csv", i_csv);
    // utils::write_file(tmp_dir + "/o.csv", o_csv);
    // std::string csv_file = tmp_dir + "/io.csv";
    std::string csv_file = builder.GetDir() + "/io.csv";
    utils::write_file(csv_file, csv);
    std::cout << "Output to: " << csv_file   << "\n";
    test_result.GetInvariants();
    test_result.GetPreconditions();
    test_result.GetTransferFunctions();

    Analyzer analyzer(csv_file);
    std::vector<std::string> invs = analyzer.GetInvariants();
    std::vector<std::string> pres = analyzer.GetPreConditions();
    std::vector<std::string> trans = analyzer.GetTransferFunctions();
    if (PrintOption::Instance()->Has(POK_AnalysisResult)) {
      std::cout << "------ invariants ------"  << "\n";
      for (auto &s : invs) {
        std::cout << "| " << s  << "\n";
      }
      std::cout << "------ pre condtions ------"  << "\n";
      for (auto &s : pres) {
        std::cout << "| " << s  << "\n";
      }
      std::cout << "------ transfer functions ------"  << "\n";
      for (auto &s : trans) {
        std::cout << "| " << s  << "\n";
      }
      std::cout << "------------------------------"  << "\n";

      // std::string cmd = "compare.py -f " + csv_file;
      // std::string inv = utils::exec(cmd.c_str());
      // std::cout << inv  << "\n";
    }


    std::cout << "---- resolveQuery -----"  << "\n";
    m_query_resolved = resolveQuery(invs, pres, trans);
    std::cout << "------ end of query resolving -----"  << "\n";
    
    /**
     * FIXME Free the space of TestInput*
     */
    for (std::vector<TestInput*> &v : test_suite) {
      for (TestInput* in : v) {
        delete in;
      }
    }

  } else {
    g_compile_error_no++;
    if (PrintOption::Instance()->Has(POK_CompileInfo)) {
      utils::print("compile error\n", utils::CK_Red);
    }
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
  }
  // TODO return good or not, to decide whether to keep the newly added statements
}



/**
 * @pram [in] output lines representing output. The format is xxx=yyy.
 * @return xxx:yyy maps
 */
std::map<std::string, std::string> get_header_value_map(std::string output) {
  std::map<std::string, std::string> ret;
  std::vector<std::string> lines = utils::split(output, '\n');
  for (std::string line : lines) {
    if (line.empty()) continue;
    // std::cout << line  << "\n";
    // assert(line.find("=") != std::string::npos);
    if (line.find("=") == std::string::npos) {
      // std::cerr << "The Line does not contain a =" << "\n";
      // std::cerr << line  << "\n";
      // assert(false);
      // FIXME sometimes the code we included from the program has output statements
      // So I just ignore such case
      // But, this may cause some hard to debug bugs
      // maybe it is a good idea to write this information to a log file for debugging
      continue;
    }
    std::string header = line.substr(0, line.find("="));
    utils::trim(header);
    std::string value = line.substr(line.find("=") + 1);
    utils::trim(value);
    ret[header] = value;
  }
  return ret;
}


/**
 * Invariants generation
 * This will parse the output.
 * The output format matters.
 * OK, I'm going to generate a CSV file, and use R to solve it.
 *
 * The output should be:
 * xxx = yyy
 * xxx != NULL
 *
 * Also, the output should be separate to test success and failure
 */
// void NewTestResult::GetInvariants() {
//   // 1. separate success and failure output
//   // std::vector<std::string> outputs = m_poi_output_success;
//   // 2. get all the headers (xxx)
//   std::vector<std::map<std::string, std::string> > header_value_maps;
//   for (std::string output : m_poi_output_success) {
//     std::map<std::string, std::string> m = get_header_value_map(output);
//     m["HELIUM_TEST_SUCCESS"] = "true";
//     header_value_maps.push_back(m);
//   }
//   for (std::string output : m_poi_output_failure) {
//     std::map<std::string, std::string> m = get_header_value_map(output);
//     m["HELIUM_TEST_SUCCESS"] = "false";
//     header_value_maps.push_back(m);
//   }
//   std::set<std::string> headers;
//   for (auto &m : header_value_maps) {
//     for (auto mm : m) {
//       headers.insert(mm.first);
//     }
//   }
//   // 3. generate the CVS file, all the unavailable fields should be marked as N/A
//   std::string csv;
//   assert(headers.size() > 0);
//   for (const std::string &header : headers) {
//     csv += header;
//     csv += ",";
//   }
//   csv.pop_back();
//   csv += "\n";

//   for (auto m : header_value_maps) {
//     // for every output, one line
//     for (const std::string &header : headers) {
//       if (m.count(header) == 1) {
//         csv += m[header] + ",";
//       } else {
//         csv += "N/A,";
//       }
//     }
//     csv.pop_back();
//     csv += "\n";
//   }
//   // 4. Call R script, generate result, in some format.
//   std::cout << csv  << "\n";
// }

void NewTestResult::GetInvariants() {
}

void NewTestResult::GetPreconditions() {
}

void NewTestResult::GetTransferFunctions() {
}

/**
 * I want the output, the precondition, to be in the same CVS file.
 * I need a data structure to hold the data.
 */
void NewTestResult::PrepareData() {
  std::set<std::string> output_headers;
  for (int i=0;i<(int)m_test_suite.size();i++) {
    // output
    std::string output = m_poi_output[i].first;
    bool success = m_poi_output[i].second;
    std::map<std::string, std::string> m = get_header_value_map(output);
    std::map<std::string, std::string> om; // added prefix "O_"
    std::map<std::string, std::string> im;
    bool poi = false;
    // add prefix "O_"
    for (auto mm : m) {
      // om["O_" + mm.first] = mm.second;
      if (mm.first[0] == 'O') {
        om[mm.first] = mm.second;
        m_o_headers.insert(mm.first);
      } else if (mm.first[0] == 'I') {
        im[mm.first] = mm.second;
        m_i_headers.insert(mm.first);
      } else {
        // HELIUM_POI
        assert(mm.first == "HELIUM_POI");
        im[mm.first] = mm.second;
        om[mm.first] = mm.second;
        m_i_headers.insert(mm.first);
        m_o_headers.insert(mm.second);
        if (mm.second == "true") poi = true;
      }
      // m_o_headers.insert("O_" + mm.first);
    }
    // input
    std::string input;
    for (TestInput *in : m_test_suite[i]) {
      input += in->ToString();
    }
    // std::cout << input  << "\n";
    m = get_header_value_map(input);
    for (auto mm : m) {
      assert(mm.first[0] == 'I');
      im[mm.first] = mm.second;
      m_i_headers.insert(mm.first);
      // im["I_" + mm.first] = mm.second;
      // m_i_headers.insert("I_" + mm.first);
    }
    // merge IO together
    m.clear();
    m.insert(im.begin(), im.end());
    m.insert(om.begin(), om.end());
    m["HELIUM_TEST_SUCCESS"] = success ? "true" : "false";
    m_i_headers.insert("HELIUM_TEST_SUCCESS");
    m_o_headers.insert("HELIUM_TEST_SUCCESS");
    m["HELIUM_POI"] = poi ? "true" : "false";
    m_i_headers.insert("HELIUM_POI");
    m_o_headers.insert("HELIUM_POI");
    m_headers.insert(m_i_headers.begin(), m_i_headers.end());
    m_headers.insert(m_o_headers.begin(), m_o_headers.end());
    m_header_value_maps.push_back(m);
  }
}

/**
 * Must be called after PrepareData
 * @param [in] io_type "I" "O" "IO"
 * @param [in] sf_type "S" "F" "SF"
 */
std::string NewTestResult::GenerateCSV(std::string io_type, std::string sf_type) {
  std::string ret;
  std::set<std::string> headers;
  assert(m_headers.size() > 0);
  assert(m_i_headers.size() > 0);
  assert(m_o_headers.size() > 0);
  // std::cout << m_headers.size()  << "\n";
  // std::cout << m_i_headers.size()  << "\n";
  // std::cout << m_o_headers.size()  << "\n";
  // different types
  if (io_type == "I") {
    // only input, preconditions
    headers = m_i_headers;
  } else if (io_type == "O") {
    // only output
    headers = m_o_headers;
  } else if (io_type == "IO") {
    headers = m_headers;
  } else {
    assert(false);
  }
  // header
  for (const std::string &header : headers) {
    ret += header;
    ret += ",";
  }
  ret.pop_back();
  ret += "\n";
  // data
  assert(sf_type == "S" || sf_type == "F" || sf_type == "SF");
  for (auto m : m_header_value_maps) {
    assert(m.count("HELIUM_TEST_SUCCESS") == 1);
    if (sf_type == "S" && m["HELIUM_TEST_SUCCESS"] == "false") continue;
    if (sf_type == "F" && m["HELIUM_TEST_SUCCESS"] == "true") continue;
    for (const std::string &header : headers) {
      if (m.count(header) == 1) {
        ret += m[header] + ",";
      } else {
        // if the record does not contains the record, give is NA
        ret += "NA,";
      }
    }
    ret.pop_back();
    ret += "\n";
  }
  return ret;
}





/********************************
 * Resolving Query
 *******************************/
/**
 * Return the inversed version of op. > becomes <
 */
std::string inverse_op(std::string op) {
  if (op == "=") return "=";
  if (op == ">") return "<";
  if (op == ">=") return "<=";
  if (op == "<") return ">";
  if (op == "<=") return ">=";
  assert(false);
}

void BinaryFormula::Inverse() {
  std::string tmp = m_rhs;
  m_rhs = m_lhs;
  m_lhs = tmp;
  m_op = inverse_op(m_op);
}


BinaryFormula::BinaryFormula(std::string raw) : m_raw(raw) {
  std::string formula = raw.substr(0, raw.find("conf:"));
  std::string conf = raw.substr(raw.find("conf:") + 5);
  m_conf = atoi(conf.c_str());
  utils::trim(formula);
  // find the <, >, <=, >=, =
  size_t pos;
  int offset;
  if (formula.find("<=") != std::string::npos) {
    pos = formula.find("<=");
    offset = 2;
    m_op = "<=";
  } else if (formula.find(">=") != std::string::npos) {
    pos = formula.find(">=");
    offset = 2;
    m_op = ">=";
  } else if (formula.find("=") != std::string::npos) {
    pos = formula.find("=");
    offset = 1;
    m_op = "=";
  } else if (formula.find("<") != std::string::npos) {
    pos = formula.find("<");
    offset = 1;
    m_op = "<";
  } else if (formula.find(">") != std::string::npos) {
    pos = formula.find(">");
    offset = 1;
    m_op = ">";
  } else {
    assert(false);
  }
  m_lhs = formula.substr(0, pos);
  m_rhs = formula.substr(pos + offset);
  utils::trim(m_lhs);
  utils::trim(m_rhs);
}



TEST(SegTestCase, BinaryFormulaTest) {
  BinaryFormula bf("Od_sizeof(tempname)=1024 conf:18");
  EXPECT_EQ(bf.GetLHS(), "Od_sizeof(tempname)");
  EXPECT_EQ(bf.GetRHS(), "1024");
  EXPECT_EQ(bf.GetOP(), "=");
  EXPECT_EQ(bf.GetConf(), 18);

  BinaryFormula bf2("Id_strlen(argv[1])<=1024");
  EXPECT_EQ(bf2.GetLHSVar(), "argv");
  std::set<std::string> ss = bf2.GetVars();
  // for (auto &s : ss) {
  //   std::cout << s  << "\n";
  // }
  ASSERT_EQ(ss.size(), 1);
  EXPECT_EQ(*ss.begin(), "argv");
}

/**
 * Get the variables.
 * for Od_strlen(*fileptr[1]), it will be fileptr along
 * Since this is binary, the return set is at most of size 2
 */
std::set<std::string> BinaryFormula::GetVars() {
  std::set<std::string> ret;
  std::string tmp;
  tmp = getVar(m_lhs);
  if (!tmp.empty()) {
    ret.insert(tmp);
  }
  tmp = getVar(m_rhs);
  if (!tmp.empty()) {
    ret.insert(tmp);
  }
  return ret;
}


std::string BinaryFormula::getVar(std::string s) {
  // remove prefix
  assert(s.size() > 3);
  // assert(s[2] == '_');
  if (s[2] != '_') return "";
  s = s.substr(3);
  // remove strlen, sizeof
  // FIXME multiple sizeof?
  if (s.find("sizeof") != std::string::npos) {
    s = s.substr(s.find("sizeof") + strlen("sizeof"));
  }
  if (s.find("strlen") != std::string::npos) {
    s = s.substr(s.find("strlen") + strlen("strlen"));
  }
  // remove ()
  while (s.find('(') != std::string::npos) {
    s.erase(s.find('('), 1);
  }
  while (s.find(')') != std::string::npos) {
    s.erase(s.find(')'), 1);
  }
  // remove [xxx]
  if (s.find('[') != std::string::npos) {
    s = s.substr(0, s.find('['));
  }
  // remove .xxx
  if (s.find('.') != std::string::npos) {
    s = s.substr(0, s.find('.'));
  }
  // remove + ...
  if (s.find('+') != std::string::npos) {
    s = s.substr(0, s.find('+'));
  }
  utils::trim(s);
  return s;
}


/**
 * Od_sizeof(tempname)=1024 conf:18
 * Od_sizeof(tempname)>=Od_strlen(*fileptr) conf: 18
 */
BinaryFormula* get_key_inv(std::vector<BinaryFormula*> invs) {
  // loop through the invs, and split into two sets: constant assignment, and other
  assert(!invs.empty());
  BinaryFormula *ret = NULL;
  std::vector<BinaryFormula*> cons;
  for (BinaryFormula *bf : invs) {
    if (bf->GetOP() == "=" && utils::is_number(bf->GetRHS())) {
      cons.push_back(bf);
    } else {
      ret = bf;
    }
  }
  assert(ret);
  for (BinaryFormula *bf : cons) {
    if (bf->GetLHS() == ret->GetRHS()) {
      ret->UpdateRHS(bf->GetRHS());
    }
    if (bf->GetLHS() == ret->GetLHS()) {
      ret->UpdateLHS(bf->GetRHS());
      ret->Inverse();
    }
  }
  return ret;
}

/**
 * Derive inv from pres and trans.
 * If cannot derive, return an empty set.
 * Otherwise return the used pre-conditions.
 * This return value is used to see if the variables are entry point.
 * TODO multiple precondition
 * TODO record the transfer function used.
 *
 * inv: Od_sizeof(tempname)>=Od_strlen(*fileptr) conf: 12
 * trans: Od_strlen(*fileptr)=Id_strlen(argv[1]) conf:12
 * pres: Id_strlen(argv[1])<=1024 conf: 12
 */
BinaryFormula* derive_key_inv(std::vector<BinaryFormula*> pres, std::vector<BinaryFormula*> trans, BinaryFormula *inv) {
  // std::cout << "drive_key_inv"  << "\n";
  // std::cout << "pres:"  << "\n";
  // for (BinaryFormula *bf : pres) {
  //   std::cout << bf->dump()  << "\n";
  // }
  // std::cout << "trans:"  << "\n";
  // for (BinaryFormula *bf : trans) {
  //   std::cout << bf->dump()  << "\n";
  // }
  // std::cout << "inv:"  << "\n";
  // std::cout << inv->dump()  << "\n";
  // randomly pair the trans and pres, to see if inv can be generated
  for (BinaryFormula *pre : pres) {
    for (BinaryFormula *tran : trans) {
      BinaryFormula tmp(*pre);
      if (tran->GetRHS() == tmp.GetLHS()) {
        tmp.UpdateLHS(tran->GetLHS());
      }
      if (tran->GetRHS() == tmp.GetRHS()) {
        tmp.UpdateRHS(tran->GetLHS());
      }
      // compare_formula(&tmp, inv);
      if (tmp.GetLHS() == inv->GetLHS() && tmp.GetRHS() == inv->GetRHS() && tmp.GetOP() == inv->GetOP()) {
        return pre;
      }
      if (tmp.GetLHS() == inv->GetRHS() && tmp.GetRHS() == inv->GetLHS() && inverse_op(tmp.GetOP()) == inv->GetOP()) {
        return pre;
      }
    }
  }
  return NULL;
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
bool Ctx::resolveQuery(std::vector<std::string> str_invs, std::vector<std::string> str_pres, std::vector<std::string> str_trans) {
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
