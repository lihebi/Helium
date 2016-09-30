#include "code_gen.h"
#include "workflow/segment.h"

#include "resolver/snippet_db.h"
#include "utils/log.h"
#include "helium_options.h"

#include <iostream>

void CodeGen::Compute() {
  std::map<AST*, std::set<ASTNode*> > ret;
  for (auto m : m_data) {
    ret[m.first] = m.first->CompleteGene(m.second);
    if (m.first == m_first_ast) {
      // std::cout << "remove root"  << "\n";
      ret[m.first] = m.first->RemoveRoot(ret[m.first]);
    }
    // comparing
    // m.first->VisualizeN(m.second, ret[m.first]);
  }


  

  
  m_data = ret;
}

std::string CodeGen::GetMain() {
  std::string ret;
  ret += get_header();
  std::string main_func;
  std::string other_func;
  /**
   * Go through all the ASTs
   * The first AST should be placed in main function.
   * Other ASTs should just output.
   */

  for (auto m : m_data) {
    AST *ast = m.first;
    std::set<ASTNode*> nodes = m.second;
    if (ast == m_first_ast) {
      main_func += "int main() {\n";
      main_func += "int helium_size;\n"; // array size of heap

      // inputs
      for (auto mm : m_inputs) {
        std::string var = mm.first;
        Type *t = mm.second;
        main_func += t->GetDeclCode(var);
        // FIXME didn't not use def use analysis result!
        main_func += t->GetInputCode(var);
      }

      main_func += "// In function " + ast->GetFunctionName() + "\n";
      main_func += "// nodes: " + std::to_string(nodes.size()) + "\n";

      // the code
      std::string code = ast->GetCode(nodes);
      
      ast->ClearDecl();
      // modify the code, specifically change all return statement to return 35;
      code = replace_return_to_35(code);
      main_func += code;
      main_func += "return 0;\n";
      main_func += "};\n";
    } else {
      std::string code = ast->GetCode(nodes);
      // for (ASTNode *node : nodes) {
      //   if (!ast->Contains(node)) {
      //     std::cout << "not contain!!!"  << "\n";
      //   }
      // }
      ast->ClearDecl();
      other_func += "// " + ast->GetFunctionName() + "\n";
      other_func += "// nodes: " + std::to_string(nodes.size()) + "\n";
      other_func += code;
      other_func += "\n";
    }

    if (HeliumOptions::Instance()->GetBool("print-segment-meta")) {
      int loc = 0;
      int branch_ct = 0;
      int loop_ct = 0;
      for (auto m : m_data) {
        std::string code = m.first->GetCode(m.second);
        for (ASTNode *node : m.second) {
          if (node->Kind() == ANK_If) {
            branch_ct++;
          }
          if (node->Kind() == ANK_While
              || node->Kind() == ANK_For
              || node->Kind() == ANK_Do) {
            loop_ct++;
          }
        }
        loc += std::count(code.begin(), code.end(), '\n');
      }
      std::cout << "Segment Size: " << loc << "\n";
      std::cout << "Procedure Number: " << m_data.size() << "\n";
      std::cout << "Branch Number: " << branch_ct << "\n";
      std::cout << "Loop Number: " << loop_ct << "\n";
    }
    if (HeliumOptions::Instance()->GetBool("print-segment-branch-number")) {
    }
  }
  
  ret += other_func;
  ret += main_func;
  return ret;
}
std::string CodeGen::GetSupport() {
  std::string code = "";

  for (auto m : m_data) {
    resolveSnippet(m.first);
  }

  
  // head
  code += get_head();
  std::set<std::string> avail_headers = SystemResolver::Instance()->GetAvailableHeaders();
  std::set<std::string> used_headers = HeaderResolver::Instance()->GetUsedHeaders();
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

std::string CodeGen::getSupportBody() {
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
  for (auto m : m_data) {
    avoid_funcs.insert(m.first->GetFunctionName());
  }
  avoid_funcs.erase(m_first_ast->GetFunctionName());
  for (std::string s : avoid_funcs) {
    std::set<int> ids = SnippetDB::Instance()->LookUp(s, {SK_Function});
    if (ids.size() == 0) {
      helium_log_warning("Function " + s + " lookup size: 0");
      continue;
    } else if (ids.size() > 1) {
      helium_log_warning("Function " + s + " lookup size: " + std::to_string(ids.size()));
    }
    int id = *ids.begin();
    std::string code = SnippetDB::Instance()->GetCode(id);
    std::string decl_code = get_function_decl(code);
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


std::string CodeGen::GetMakefile() {
  std::string makefile;
  std::string cc = HeliumOptions::Instance()->GetString("cc");
  makefile += "CC:=" + cc + "\n";
  // TODO test if $CC is set correctly
  // makefile += "type $(CC) >/dev/null 2>&1 || { echo >&2 \"I require $(CC) but it's not installed.  Aborting.\"; exit 1; }\n";
  makefile += ".PHONY: all clean test\n";
  makefile = makefile + "a.out: main.c\n"
    + "\t$(CC) -g "
    // comment out because <unistd.h> will not include <optarg.h>
    // + "-std=c11 "
    + "main.c "
    + (HeliumOptions::Instance()->GetBool("address-sanitizer") ? "-fsanitize=address " : "")
    // gnulib should not be used:
    // 1. Debian can install it in the system header, so no longer need to clone
    // 2. helium-lib already has those needed headers, if installed correctly by instruction
    // + "-I$(HOME)/github/gnulib/lib " // gnulib headers
    +
    (HeliumOptions::Instance()->GetBool("gnulib") ?
     "-I$(HOME)/github/helium-lib " // config.h
     "-I$(HOME)/github/helium-lib/lib " // gnulib headers
     "-L$(HOME)/github/helium-lib/lib -lgnu " // gnulib library
     : "")
    + "-I/usr/include/x86_64-linux-gnu " // linux headers, stat.h
    + "-fprofile-arcs -ftest-coverage " // gcov coverage
    + SystemResolver::Instance()->GetLibs() + "\n"
    + "clean:\n"
    + "\trm -rf *.out\n"
    + "test:\n"
    + "\tbash test.sh";
    
  return makefile;
}




void CodeGen::resolveSnippet(AST *ast) {
  helium_print_trace("Context::resolveSnippet");
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

  std::set<ASTNode*> nodes = m_data[ast];
  for (ASTNode *n : nodes) {
    assert(n);
    std::set<std::string> ids = n->GetIdToResolve();


    // if (ids.count("lzw")==1) {
    //   std::cout << n->GetLabel()  << "\n";
    //   ast->VisualizeN(nodes, {});
    //   getchar();
    // }


    all_ids.insert(ids.begin(), ids.end());
  }

  // I need to remove the function here!
  // otherwise the code snippet will get too much
  // For example, I have included a function in main.c, but it turns out to be here
  // even if I filter it out when adding to support.h, I still have all its dependencies in support.h!
  for (auto m : m_data) {
    AST *ast = m.first;
    assert(ast);
    std::string func = ast->GetFunctionName();
    all_ids.erase(func);
  }


  // std::cout << "all ids for snippet: "  << "\n";
  // for (std::string id : all_ids) {
  //   std::cout <<id  << ",";
  // }
  // std::cout   << "\n";

  std::set<int> snippet_ids = SnippetDB::Instance()->LookUp(all_ids);
  // not sure if it should be here ..
  std::set<int> all_snippet_ids = SnippetDB::Instance()->GetAllDep(snippet_ids);
  all_snippet_ids = SnippetDB::Instance()->RemoveDup(all_snippet_ids);
  m_snippet_ids.insert(all_snippet_ids.begin(), all_snippet_ids.end());
  helium_print_trace("Context::resolveSnippet end");
}
