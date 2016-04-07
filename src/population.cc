#include "population.h"
#include "segment.h"
#include <iostream>
#include "snippet_db.h"
#include "options.h"

void Individual::RandGene() {
  if (m_gene) delete m_gene;
  m_gene = new Gene(m_ast);
  // m_gene->Rand(m_ast->size());
  m_gene->LeafRand();
}

void Population::CreateRandomIndividuals(int num) {
  print_trace("Population::CreateRandomIndividuals");
  for (int i=0;i<num;i++) {
    Individual *ind  = new Individual(m_ast);
    ind->RandGene();
    m_individuals.push_back(ind);
  }
}


/**
 * Always call Complete before you do any computation for population.
 * Do it right after you generate some gene, possibly by RandGene()
 */
// void Population::Complete() {
//   for (Gene *g : m_genes) {
//     Gene cg = m_ast->CompleteGene(*g);
//     // FIXME deprecated
//     Gene defuseg = m_ast->CompleteVarDefUse(cg);
//     m_cgene_m[g] = cg;
//     m_defuse_gene_m[g] = defuseg;
//     std::set<int> indice = defuseg.GetIndiceS();
//     if (indice.count(0) == 1) {
//       // contains function def
      
//       ASTNode *func = m_ast->GetNodeByIndex(0);
//       m_function_m[g] = dynamic_cast<Function*>(func)->GetName();
//     }
//   }
// }

void Population::Solve() {
  print_trace("Population::Solve()");
  for (Individual *ind : m_individuals) {
    ind->Solve();
  }
}

void Individual::Visualize() {
  if (m_gene) {
    m_ast->VisualizeI(m_gene->GetIndiceS(), {});
  }
}

std::string Individual::GetMain() {
  print_trace("Individual::GetMain()");
  // segment must in here
  // add a comment before seg
  std::string ret;
  ret += get_header();
  // if (indice.count(0) == 1) {
  if (m_gene->HasIndex(0)) {
    // the function node is selected, thus it should not be placed in the main function.
    ret += GetCode();
    if (m_ast->GetFunctionName() != "main") {
      ret += "int main() {\n";
      // TODO need to call that function
      ret += "return 0;";
      ret += "};";
    }
  } else {
    ret += "int main() {\n";
    ret += GetCode();
    ret += "\nreturn 0;";
    ret += "\n}";
  }
  // restore
  return ret;
}

std::string Individual::GetCode() {
  print_trace("Individual::GetCode()");
  m_ast->SetDecl(m_decl_input_m, m_decl_m);
  std::string ret = m_ast->GetCode(m_gene->GetIndiceS());
  m_ast->ClearDecl();
  return ret;
}

std::string Individual::GetMakefile() {
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

/**
 * Solve the population problem.
 * Will be a recursive worklist algorithm
 * 1. compute the LCA
 * 3. complete LCA, and form the worklist
 * 2. for every node in the worklist, find the var name used
 *   2.1 find their declaration and definition
 *   2.2 If the definition is found, and does not use itself
 *      2.2.1 add it to the nodes
 *      2.2.2 complete for the node
 *      2.2.3 add the declaration at the decl node
 *   2.3 Otherwise, add the decl and input at the decl node
 */
void Individual::Solve() {
  print_trace("Individual::Solve()");
  std::set<ASTNode*> newnodes = m_ast->CompleteGene(m_gene);
  std::set<ASTNode*> worklist = m_gene->ToASTNodeSet();
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
    std::set<std::string> ids = node->GetVarIds();
    for (std::string id : ids) {
      if (id.empty()) continue;
      if (is_c_keyword(id)) continue;
      // std::cout <<id << " YYY at " << m_idx_m[node] << "\n";
      SymbolTable *tbl = node->GetSymbolTable();
      SymbolTableValue *decl = tbl->LookUp(id);
      ASTNode *def = node->LookUpDefinition(id);
      if (!decl) continue;
      if (def) {
        // need this def, but not necessary to be a new node
        // and also possibly need the decl
        m_gene->AddNode(def);
        worklist.insert(def);
        std::set<ASTNode*> morenodes = m_ast->CompleteGene(m_gene);
        worklist.insert(morenodes.begin(), morenodes.end());
        decl_m[decl->GetNode()].insert(id);
      } else {
        // may need decl, and need input
        decl_input_m[decl->GetNode()].insert(id);
      }
    }
  }
  // remove from the maps those that already in gene
  for (ASTNode *n : done) {
    decl_input_m.erase(n);
    decl_m.erase(n);
  }
  // store these two maps
  m_decl_input_m = decl_input_m;
  m_decl_m = decl_m;
  // resolve
  ResolveSnippet();
}
void Individual::ResolveSnippet() {
  print_trace("Individual::ResolveSnippet()");
  std::set<std::string> all_ids;
  std::map<ASTNode*, std::set<std::string> > all_decls;
  all_decls.insert(m_decl_input_m.begin(), m_decl_input_m.end());
  all_decls.insert(m_decl_m.begin(), m_decl_m.end());
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
  std::set<ASTNode*> nodes = m_gene->ToASTNodeSet();
  for (ASTNode *n : nodes) {
    std::set<std::string> ids = n->GetIdToResolve();
    all_ids.insert(ids.begin(), ids.end());
  }
  m_snippet_ids = snippetdb::look_up_snippet(all_ids);
  // not sure if it should be here ..
  m_all_snippet_ids = snippetdb::get_all_dependence(m_snippet_ids);
}


std::string Individual::GetSupport() {
  print_trace("Individual::GetSupport()");
  // m_all_snippet_ids = snippetdb::get_all_dependence(m_snippet_ids);
  std::vector<int> sorted_snippet_ids = snippetdb::sort_snippets(m_all_snippet_ids);
  std::string code = "";
  // head
  code += get_head();
  code += SystemResolver::Instance()->GetHeaders();
  code += "\n/****** codes *****/\n";
  // snippets
  std::string code_func_decl;
  std::string code_func;
  std::string avoid_func;
  // if (m_function_m.count(m_genes[idx])==1) {
  //   avoid_func = m_function_m[m_genes[idx]];
  // }
  if (m_gene->HasIndex(0)) {
    // the function node is selected.
    // the function name should be avoided
    avoid_func = m_ast->GetFunctionName();
  }
  // std::cout << sorted_snippet_ids.size()  << "\n";
  for (int id : sorted_snippet_ids) {
    snippetdb::SnippetMeta meta = snippetdb::get_meta(id);
    if (meta.HasKind(SK_Function)) {
      std::string func = meta.GetKey();
      if (avoid_func != func) {
        code_func += "/* " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
        code_func += snippetdb::get_code(id) + '\n';
        code_func_decl += get_function_decl(snippetdb::get_code(id))+"\n";
      }
    } else {
      // every other support code(structures) first
      code += "/* " + meta.filename + ":" + std::to_string(meta.linum) + "*/\n";
      code += snippetdb::get_code(id) + '\n';
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


#if 0
void Individual::ResolveSnippet() {
  std::map<ASTNode*, std::set<std::string> > all_decls;
  all_decls.insert(m_decl_input_m.begin(), m_decl_input_m.end());
  all_decls.insert(m_decl_m.begin(), m_decl_m.end());
  for (auto item : all_decls) {
    ASTNode *node = item.first;
    std::set<std::string> names = item.second;
    for (std::string name : names) {
      SymbolTableValue *value = node->GetSymbolTable()->LookUp(name);
      std::string type = value->GetType();
      std::set<std::string> ids = extract_id_to_resolve(type);
      for (std::string id : ids) {
        std::set<Snippet*> snippets = SnippetRegistry::Instance()->Resolve(id);
        m_snippets.insert(snippets.begin(), snippets.end());
      }
    }
  }
  // resolve the nodes selected by gene
  std::set<ASTNode*> nodes = m_gene->ToASTNodeSet();
  for (ASTNode *n : nodes) {
    std::set<std::string> ids = n->GetIdToResolve();
    // std::cout << "ID: " << ids.size()  << "\n";
    for (std::string id : ids) {
      // std::cout << id  << "\n";
      std::set<Snippet*> snippets = SnippetRegistry::Instance()->Resolve(id);
      m_snippets.insert(snippets.begin(), snippets.end());
    }
  }
}
#endif


#if 0
std::string Individual::GetSupport() {
  // prepare the containers
  std::set<Snippet*> all_snippets;
  all_snippets = SnippetRegistry::Instance()->GetAllDeps(GetSnippets());
  // sort the snippets
  std::vector<Snippet*> sorted_all_snippets = sort_snippets(all_snippets);
  // FIXME This sorted is 0
  // return the snippet code
  std::string code = "";
  // head
  code += get_head();
  code += SystemResolver::Instance()->GetHeaders();
  code += "\n/****** codes *****/\n";
  // snippets
  std::string code_func_decl;
  std::string code_func;
  std::string avoid_func;
  // if (m_function_m.count(m_genes[idx])==1) {
  //   avoid_func = m_function_m[m_genes[idx]];
  // }
  if (m_gene->HasIndex(0)) {
    // the function node is selected.
    // the function name should be avoided
    avoid_func = m_ast->GetFunctionName();
  }
  for (Snippet* s : sorted_all_snippets) {
    if (s->MainKind() == SK_Function) {
      if (avoid_func != s->MainName()) {
        code_func += "/* " + s->GetFileName() + ":" + std::to_string(s->GetLineNumber()) + "*/\n";
        code_func += s->GetCode() + '\n';
        code_func_decl += get_function_decl(s->GetCode())+"\n";
      }
    } else {
      // every other support code(structures) first
      code += "/* " + s->GetFileName() + ":" + std::to_string(s->GetLineNumber()) + "*/\n";
      code += s->GetCode() + '\n';
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
#endif
