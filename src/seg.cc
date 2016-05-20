#include "seg.h"
#include "options.h"
#include <iostream>
#include "utils.h"
#include "snippet_db.h"
#include "xml_doc_reader.h"

#include "segment.h" // for get_header, etc
#include "resolver.h" // for SystemResolver
#include "builder.h"
using namespace ast;

Seg::Seg(ast::XMLNode xmlnode) {
  print_trace("Seg::Seg()");
  XMLNode function_node = get_function_node(xmlnode);
  assert(function_node);
  std::string func_name = function_get_name(function_node);
  // AST *ast = getAST(function_node);
  AST *ast = new AST(function_node);
  m_func_to_ast_m[func_name] = ast;
  m_asts.push_back(ast);
  // POI
  // FIXME Not sure if I should use GetEnclosingNodeByXMLNode
  ASTNode *astnode = ast->GetNodeByXMLNode(xmlnode);
  m_nodes.insert(astnode);
  // Initial context
  m_ctxs.push_back(new Ctx(this));
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
  Ctx *last = m_ctxs.back();
  Ctx *ctx = new Ctx(*last);
  m_ctxs.push_back(ctx);
  ASTNode *first_node = ctx->GetFirstNode();
  assert(first_node);
  assert(first_node->GetAST());
  ASTNode *leaf = first_node->GetAST()->GetPreviousLeafNode(first_node);
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
  } else {
    // std::cerr << "Inter procedure"  << "\n";
    utils::print("Inter procedure\n", utils::CK_Red);
    AST *ast = first_node->GetAST();
    if (ast->GetFunctionName() == "main") {
      utils::print("reach beginning of main. Stop.\n", utils::CK_Green);
      return false;
    }
    XMLDoc *doc = createCallerDoc(ast);
    // reach the function def
    // query the callgraph, find call-sites, try one callsite.
    // from that, form the new context
    AST *newast = ASTFactory::CreateASTFromDoc(doc);
    assert(newast);
    m_asts.push_back(newast);
    m_func_to_ast_m[newast->GetFunctionName()] = newast;
    // get callsite node
    Node callsite_xmlnode = find_callsite(*doc, ast->GetFunctionName());
    ASTNode *callsite = newast->GetEnclosingNodeByXMLNode(callsite_xmlnode);
    assert(callsite);
    assert(callsite->GetAST());
    ctx->SetFirstNode(callsite);
    ctx->AddNode(callsite);
  }
  ctx->Resolve();
  // ctx->dump();
  ctx->Test();
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
  std::string func_code = SnippetDB::Instance()->GetCode(caller_id);
  XMLDoc *doc = XMLDocReader::CreateDocFromString(func_code);
  m_docs.push_back(doc);
  return doc;
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
    if (ast != first_ast) {
      // this is inner function.
      // this one should not have input variables.
      // all the inputs should be resolved such that the input is self-sufficient
      // Complete to root to include the function prototype
      // FIXME can I modify the map here?
      // TODO complete def use
      std::set<ASTNode*> complete;
      complete = resolveDecl(ast, true);
      m_ast_to_node_m[ast] = complete;
    } else {
      // this is the first AST
      // this one needs input variables
      std::set<ASTNode*> complete;
      complete = resolveDecl(ast, false);
      // Remove root for the outmost AST!
      complete = ast->RemoveRoot(complete);
      m_ast_to_node_m[ast] = complete;
    }
    resolveSnippet(ast);
  }
}


std::set<ASTNode*> Ctx::resolveDecl(AST *ast, bool to_root) {
  print_trace("Ctx::resolveDecl");
  std::set<ASTNode*> ret = m_ast_to_node_m[ast];
  if (to_root) {
    ret = ast->CompleteGeneToRoot(ret);
  } else {
    ret = ast->CompleteGene(ret);
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
        // need this def, but not necessary to be a new node
        // and also possibly need the decl
        ret.insert(def);
        worklist.insert(def);
        std::set<ASTNode*> morenodes = ast->CompleteGene(ret);
        worklist.insert(morenodes.begin(), morenodes.end());
        decl_m[decl->GetNode()].insert(id);
      } else {
        // may need decl, and need input
        decl_input_m[decl->GetNode()].insert(id);
      }
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
  return ret;
}

/**
 * TODO
 */
void Ctx::resolveSnippet(AST *ast) {
  std::set<std::string> all_ids;
  std::map<ASTNode*, std::set<std::string> > all_decls;
  decl_deco decl_m = m_ast_to_deco_m[ast].first;
  decl_deco decl_input_m = m_ast_to_deco_m[ast].second;
  all_decls.insert(decl_input_m.begin(), decl_input_m.end());
  all_decls.insert(decl_m.begin(), decl_m.end());
  for (auto item : all_decls) {
    ASTNode *node = item.first;
    std::set<std::string> names = item.second;
    for (std::string name : names) {
      SymbolTableValue *value = node->GetSymbolTable()->LookUp(name);
      std::string type = value->GetType();
      std::set<std::string> ids = extract_id_to_resolve(type);
      all_ids.insert(ids.begin(), ids.end());
    }
  }
  // resolve the nodes selected by gene
  std::set<ASTNode*> nodes = m_ast_to_node_m[ast];
  for (ASTNode *n : nodes) {
    std::set<std::string> ids = n->GetIdToResolve();
    all_ids.insert(ids.begin(), ids.end());
  }
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
      // TODO need to call that function
      ast->SetDecl(m_ast_to_deco_m[ast].second, m_ast_to_deco_m[ast].first);
      ast->SetDecoDecl(m_ast_to_deco_m[ast].first);
      // std::cout << "deco size: "  << "\n";
      // std::cout << m_ast_to_deco_m[ast].second.size()  << "\n";
      ast->SetDecoDeclInput(m_ast_to_deco_m[ast].second);
      std::string code = ast->GetCode(nodes);
      ast->ClearDecl();
      main_func += code;
      main_func += "return 0;";
      main_func += "};";
    } else {
      // other functions
      ast->SetDecl(m_ast_to_deco_m[ast].second, m_ast_to_deco_m[ast].first);
      ast->SetDecoDecl(m_ast_to_deco_m[ast].first);
      ast->SetDecoDeclInput(m_ast_to_deco_m[ast].second);
      std::string code = ast->GetCode(nodes);
      ast->ClearDecl();
      other_func += code;
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
  std::string code_func;
  /**
   * Avoid all the functions
   */
  std::set<std::string> avoid_funcs;
  for (auto m : m_ast_to_node_m) {
    avoid_funcs.insert(m.first->GetFunctionName());
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
      }
    } else {
      // every other support code(structures) first
      code += "/* ID: " + std::to_string(id) + " " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
      code += SnippetDB::Instance()->GetCode(id) + '\n';
    }
  }

  code += "\n// function declarations\n";
  code += code_func_decl;
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


void Ctx::Test() {
  Builder builder;
  builder.SetMain(getMain());
  builder.SetSupport(getSupport());
  builder.SetMakefile(getMakefile());
  builder.Write();
  if (PrintOption::Instance()->Has(POK_CodeOutputLocation)) {
    std::cout << "Code output to "  << builder.GetDir() << "\n";
  }
  builder.Compile();
  if (builder.Success()) {
    g_compile_success_no++;
    if (PrintOption::Instance()->Has(POK_CompileInfo)) {
      utils::print("success\n", utils::CK_Green);
    }
    if (PrintOption::Instance()->Has(POK_CompileInfoDot)) {
      utils::print(".", utils::CK_Green);
      std::cout << std::flush;
    }
  } else {
    g_compile_error_no++;
    if (PrintOption::Instance()->Has(POK_CompileInfo)) {
      utils::print("error\n", utils::CK_Red);
    }
    if (PrintOption::Instance()->Has(POK_CompileInfoDot)) {
      utils::print(".", utils::CK_Red);
      std::cout << std::flush;
    }
    if (DebugOption::Instance()->Has(DOK_PauseCompileError)) {
      std::cout <<".. print enter to continue .."  << "\n";
      getchar();
    }
  }
  // TODO return good or not, to decide whether to keep the newly added statements
}
