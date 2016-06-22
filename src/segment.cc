#include "segment.h" // for get_header, etc
#include "options.h"
#include <iostream>
#include "utils.h"
#include "snippet_db.h"
#include "xml_doc_reader.h"
#include "slice_reader.h"

#include "resolver.h" // for SystemResolver
#include "builder.h"
#include "global_variable.h"
#include "config.h"
#include "analyzer.h"
#include <gtest/gtest.h>

#include "context.h"

using namespace ast;
using namespace utils;

std::string
get_head() {
  return
    "#ifndef __SUPPORT_H__\n"
    "#define __SUPPORT_H__\n"
    "typedef int bool;\n"
    "#define true 1\n"
    "#define false 0\n"
    // some code will config to have this variable.
    // Should not just define it randomly, but this is the best I can do to make it compile ...
    "#define VERSION \"1\"\n"
    "#define PACKAGE \"helium\"\n"
    // GNU Standard
    "#define PACKAGE_NAME \"helium\"\n"
    "#define PACKAGE_BUGREPORT \"xxx@xxx.com\"\n"
    "#define PACKAGE_STRING \"helium 0.1\"\n"
    "#define PACKAGE_TARNAME \"helium\"\n"
    "#define PACKAGE_URL \"\"\n"
    "#define PACKAGE_VERSION \"0.1\"\n"
    // unstandard primitive typedefs, such as u_char
    "typedef unsigned char u_char;\n"
    "typedef unsigned int u_int;\n"

    "#define HELIUM_ASSERT(cond) if (!(cond)) exit(1)\n"
    ;
}

std::string
get_foot() {
  return std::string()
    + "\n#endif\n";
}

std::string get_header() {
  std::string s;
  std::vector<std::string> system_headers;
  std::vector<std::string> local_headers;
  system_headers.push_back("stdio.h");
  local_headers.push_back("support.h");
  for (auto it=system_headers.begin();it!=system_headers.end();it++) {
    s += "#include <" + *it + ">\n";
  }
  for (auto it=local_headers.begin();it!=local_headers.end();it++) {
    s += "#include \"" + *it + "\"\n";
  }
  return s;
}




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
std::string replace_return_to_35(const std::string &code) {
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
 * FIXME currently it is only used in output variable resolving
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

Segment::Segment(ast::XMLNode xmlnode) {
  print_trace("Segment::Segment()");
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
  // m_ctxs.push_back(new Context(this));
}
Segment::~Segment() {
  for (ast::AST *ast : m_asts) {
    delete ast;
  }
  for (ast::XMLDoc* doc : m_docs) {
    delete doc;
  }
  for (Context *ctx : m_ctxs) {
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
bool Segment::NextContext() {
  print_trace("Segment::NextContext()");
  Context *ctx;
  if (m_ctxs.empty()) {
    ctx = new Context(this);
    m_ctxs.push_back(ctx);
    ctx->Resolve();
    ctx->Test();
    return true;
  }
  Context *last = m_ctxs.back();
  ctx = new Context(*last);
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

ast::XMLDoc* Segment::createCallerDoc(AST *ast) {
  print_trace("Segment::createCallerDoc()");
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
ast::XMLNode Segment::getCallerNode(AST *ast) {
  print_trace("Segment::createCallerDoc()");
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
// AST* Segment::getAST(XMLNode function_node) {
//   assert(function_node && kind(function_node) == NK_Function);
//   if (m_asts_m.count(function_node) == 0) {
//     m_asts_m[function_node] = new AST(function_node);
//   }
//   return m_asts_m[function_node];
// }







