#include "generator.h"

#include "resolver/snippet_db.h"
#include "utils/log.h"
#include "helium_options.h"
#include "utils/utils.h"
#include "parser/xml_doc_reader.h"
#include "type/io_helper.h"


#include <iostream>



std::string
get_head() {
  return
    "#ifndef __SUPPORT_H__\n"
    "#define __SUPPORT_H__\n"

    // suppress the warning
    // "typedef int bool;\n"
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

    // FIXME why I need these?
    "typedef unsigned char uchar;\n"
    "typedef unsigned int uint;\n"

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
  system_headers.push_back("stdlib.h");
  system_headers.push_back("string.h");
  local_headers.push_back("main.h");
  for (auto it=system_headers.begin();it!=system_headers.end();it++) {
    s += "#include <" + *it + ">\n";
  }
  for (auto it=local_headers.begin();it!=local_headers.end();it++) {
    s += "#include \"" + *it + "\"\n";
  }

  if (HeliumOptions::Instance()->GetBool("gcov-handle-sigsegv")) {
    s += "#include <signal.h>\n";
    s += R"prefix(
void __gcov_flush(void);
void sigsegv_handler(int signum, siginfo_t *info, void *data) {
  printf("Received signal finally\n");
  __gcov_flush();
}
#define SEGV_STACK_SIZE BUFSIZ
)prefix";
  }
  return s;
}


/**
 * On xml node (pugixml), remove node, add a new node with tag name "tagname", and pure text value content
 * The node should NOT be the root node, otherwise assertion failure will occur.
 */
static void replace_xml_node(XMLNode node, std::string tagname, std::string content) {
  assert(node.parent());
  XMLNode new_node = node.parent().insert_child_before(tagname.c_str(), node);
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
  std::string ret = get_text(root);
  delete doc;
  return ret;
}


// void CodeGen::Compute() {
//   std::map<AST*, std::set<ASTNode*> > ret;
//   for (auto m : m_data) {
//     ret[m.first] = m.first->CompleteGene(m.second);
//     if (m.first == m_first_ast) {
//       // std::cout << "remove root"  << "\n";
//       ret[m.first] = m.first->RemoveRoot(ret[m.first]);
//     }
//     // comparing
//     // m.first->VisualizeN(m.second, ret[m.first]);
//   }
//   m_data = ret;
// }

/**
 * Called before GetXXX and after setting all nodes, inputs.
 * Compute the Switch case to be included.
 */
// void CodeGen::Preprocess() {
//   // call PatchGrammar on nodes
//   if (m_first_ast && m_data.count(m_first_ast) == 1) {
//     std::set<ASTNode*> nodes = m_data[m_first_ast];
//     nodes = m_first_ast->PatchGrammar(nodes);
//     m_data[m_first_ast] = nodes;
//   }
// }


std::string CodeGen::GetMain() {
  std::string ret;
  ret += get_header();
  // ret += Type::GetHeader();
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
      main_func += "\n";
      main_func += "\n"; // array size of heap


      // heap address
      // global variable

      main_func += R"prefix(
int main() {
  int helium_size;
)prefix";

      if (HeliumOptions::Instance()->GetBool("gcov-handle-sigsegv")) {
        main_func += R"prefix(
  struct sigaction action;
  bzero(&action, sizeof(action));
  action.sa_flags = SA_SIGINFO|SA_STACK;
  action.sa_sigaction = &sigsegv_handler;
  sigaction(SIGSEGV, &action, NULL);


  stack_t segv_stack;
  segv_stack.ss_sp = valloc(SEGV_STACK_SIZE);
  segv_stack.ss_flags = 0;
  segv_stack.ss_size = SEGV_STACK_SIZE;
  sigaltstack(&segv_stack, NULL);
)prefix";
      }

      main_func += "printf(\"HELIUM_INPUT_CODE\\n\");\n" + flush_output;
      // inputs
      for (Variable *var : m_inputs) {
        main_func += var->GetDeclCode();
        if (HeliumOptions::Instance()->GetBool("instrument-io")) {
          main_func += var->GetInputCode();
        }
      }

      main_func += "printf(\"HELIUM_INPUT_SPEC\\n\");\n" + flush_output;
      if (HeliumOptions::Instance()->GetBool("instrument-io")) {
        for (Variable *var : m_inputs) {
          main_func += var->GetOutputCode();
        }
      } else {
        main_func += "// instrument-io turned off\n";
      }
      main_func += "printf(\"HELIUM_INPUT_SPEC_END\\n\");\n" + flush_output;

      main_func += "// In function " + ast->GetFunctionName() + "\n";
      main_func += "// nodes: " + std::to_string(nodes.size()) + "\n";

      // the code
      std::string code = ast->GetCode(nodes);
      
      ast->ClearDecl();
      // modify the code, specifically change all return statement to return 35;
      code = replace_return_to_35(code);
      main_func += code;
      main_func += "return 0;\n";
      main_func += "}\n";
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
  }
  
  ret += other_func;
  ret += main_func;

  if (HeliumOptions::Instance()->GetBool("print-segment-meta")) {
    int loc = 0;
    int branch_ct = 0;
    int loop_ct = 0;
    int ast_node_ct = 0;
    for (auto m : m_data) {
      for (ASTNode *node : m.second) {
        ast_node_ct++;
        if (node->Kind() == ANK_If) {
          branch_ct++;
        }
        if (node->Kind() == ANK_While
            || node->Kind() == ANK_For
            || node->Kind() == ANK_Do) {
          loop_ct++;
        }
      }
      ASTOption::TurnOffPOIInstrument();
      std::string code = m.first->GetCode(m.second);
      ASTOption::TurnOnPOIInstrument();
      loc += std::count(code.begin(), code.end(), '\n');
    }
    std::cout << utils::PURPLE << "Segment Meta:" << utils::RESET << "\n";
    std::cout << "\t" << "AST Node Number: " << ast_node_ct << "\n";
    std::cout << "\t" << "Segment Size (LOC): " << loc << "\n";
    std::cout << "\t" << "Procedure Number: " << m_data.size() << "\n";
    std::cout << "\t" << "Branch Number: " << branch_ct << "\n";
    std::cout << "\t" << "Loop Number: " << loop_ct << "\n";
  }

  if (HeliumOptions::Instance()->GetBool("print-segment-peek")) {
    std::cout << "-- Segment Peek:" << "\n";

    for (auto m : m_data) {
      AST *ast = m.first;
      std::set<ASTNode*> nodes = m.second;
      if (ast == m_first_ast) {
        ASTOption::TurnOffPOIInstrument();
        std::string code = m.first->GetCode(m.second);
        std::cout << utils::BLUE << utils::indent_string(code) << utils::RESET << "\n";
        ASTOption::TurnOnPOIInstrument();
      }
    }

    // print up to 3 lines
    // int loc = HeliumOptions::Instance()->GetInt("segment-peek-loc");
    // if (std::count(code.begin(), code.end(), '\n') <= loc) {
    //   std::cout << code << "\n";
    // } else {
    //   int idx = 0;
    //   while (loc-- > 0) {
    //     int idx_new = code.find('\n', idx);
    //     std::string tmp = code.substr(idx, idx_new);
    //     std::cout << tmp << "\n";
    //     idx = idx_new+1;
    //   }
    // }
    std::cout << "-- Segment Peek end" << "\n";

  }

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

  // some predefined headers ...
  // I'm adding <getopt.h> because polymorph does not include it in its files ...
  // should be a better idea if checking if these headers exist on current machine
  code += R"prefix(
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
)prefix";
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



  // Adding libhelium here
  code += "\n// libhelium222\n";
  code += R"prefix(
void *hhaddr[BUFSIZ];
int hhsize[BUFSIZ];
int hhtop = 0;
char hbuf[BUFSIZ];
)prefix";
  code += IOHelper::Instance()->GetAll();
  
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
    + "-std=c11 "
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
    + "\trm -rf *.out *.gcda *.gcno\n"
    + "test:\n"
    + "\tbash test.sh";
    
  return makefile;
}




void CodeGen::resolveSnippet(AST *ast) {
  helium_print_trace("Context::resolveSnippet");
  std::set<std::string> all_ids;
  std::map<ASTNode*, std::set<std::string> > all_decls;
  /**
   * I want to resolve the type of the decls
   * And also the char array[MAX_LENGTH], see that macro?
   */

  for (Variable *v : m_inputs) {
    Type *t = v->GetType();
    if (t) {
      std::string raw = t->GetRaw();
      // resolve it
      std::set<std::string> ids = extract_id_to_resolve(raw);
      all_ids.insert(ids.begin(), ids.end());
    }
  }

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
  // even if I filter it out when adding to main.h, I still have all its dependencies in main.h!
  for (auto m : m_data) {
    AST *ast = m.first;
    assert(ast);
    std::string func = ast->GetFunctionName();
    all_ids.erase(func);
  }


  if (HeliumOptions::Instance()->Has("verbose")) {
    std::cout << "Resolved IDs:" << "\n";
    for (std::string id : all_ids) {
      std::cout << id << " ";
    }
    std::cout << "\n";
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
