#include "population.h"
#include "segment.h"
#include <iostream>

void Population::RandGenes(int num) {
  if (!m_ast) return;
  clearGene();
  for (int i=0;i<num;i++) {
    Gene *g = new Gene();
    g->Rand(m_ast->size());
    m_genes.push_back(g);
  }
}

std::string Population::GetCode(size_t idx) {
  if (idx >= m_genes.size()) return "";
  assert(!m_defuse_gene_m.empty());
  std::set<ASTNode*> nodes
    = m_ast->Index2Node(m_defuse_gene_m[m_genes[idx]].GetIndice());
  return m_ast->GetCode(nodes);
}

/**
 * Always call Complete before you do any computation for population.
 * Do it right after you generate some gene, possibly by RandGene()
 */
void Population::Complete() {
  for (Gene *g : m_genes) {
    Gene cg = m_ast->CompleteGene(*g);
    Gene defuseg = m_ast->CompleteVarDefUse(cg);
    m_cgene_m[g] = cg;
    m_defuse_gene_m[g] = defuseg;
    std::set<int> indice = defuseg.GetIndiceS();
    if (indice.count(0) == 1) {
      // contains function def
      
      ASTNode *func = m_ast->GetNodeByIndex(0);
      m_function_m[g] = dynamic_cast<Function*>(func)->GetName();
    }
  }
}

void Population::Visualize(size_t idx) {
  if (idx >= m_genes.size()) return;
  assert(!m_defuse_gene_m.empty());
  Gene *g = m_genes[idx];
  m_ast->VisualizeI(g->GetIndiceS(), {}, std::to_string(idx));
  // Gene *g = m_genes[idx];
  // Gene cg = m_ast->CompleteGene(*g);
  m_ast->VisualizeI(m_cgene_m[g].GetIndiceS(), {}, std::to_string(idx)+"complete");
  // Gene defuseg = m_ast->CompleteVarDefUse(cg);
  m_ast->VisualizeI(m_defuse_gene_m[g].GetIndiceS(), {}, std::to_string(idx) + "defuse");
}

std::string Population::GetMain(size_t idx) {
  if (idx >= m_genes.size()) return "";
  assert(!m_defuse_gene_m.empty());
  // segment must in here
  // add a comment before seg
  std::string ret;
  ret += get_header();

  Gene *g = m_genes[idx];
  // std::set<int> indice = m_defuse_gene_m[g].GetIndiceS();
  // if (indice.count(0) == 1) {
  if (m_function_m.count(g)) {
    // contains function def
    ret += GetCode(idx);
    ret += "int main() {\n";
    // TODO call the func
    ret += "\nreturn 0;";
    ret += "\n}";
  } else {
    ret += "int main() {\n";
    ret += GetCode(idx);
    ret += "\nreturn 0;";
    ret += "\n}";
  }
  // restore
  return ret;
}

std::string Population::GetMakefile() {
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

void Population::ResolveSnippet() {
  for (Gene *g : m_genes) {
    Gene defuseg = m_defuse_gene_m[g];
    std::set<ASTNode*> nodes = m_ast->Index2Node(defuseg.GetIndice());
    // std::cout << "Node: " << nodes.size()  << "\n";
    for (ASTNode *n : nodes) {
      std::set<std::string> ids = n->GetIdToResolve();
      // std::cout << "ID: " << ids.size()  << "\n";
      for (std::string id : ids) {
        // std::cout << id  << "\n";
        std::set<Snippet*> snippets = SnippetRegistry::Instance()->Resolve(id);
        m_snippets[g].insert(snippets.begin(), snippets.end());
      }
    }
  }
}

std::string Population::GetSupport(size_t idx) {
  // prepare the containers
  std::set<Snippet*> all_snippets;
  all_snippets = SnippetRegistry::Instance()->GetAllDeps(GetSnippets(idx));
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
  if (m_function_m.count(m_genes[idx])==1) {
    avoid_func = m_function_m[m_genes[idx]];
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
