#include "reader.h"

#include <stdio.h>

#include <cstring>
#include <signal.h>
#include <setjmp.h>
#include <iostream>

#include "config.h"
#include "builder.h"
#include "tester.h"
#include "analyzer.h"
#include "type.h"

#include "utils.h"

#include <gtest/gtest.h>

using namespace utils;
using namespace ast;

int Reader::m_skip_segment = -1;
int Reader::m_cur_seg_no = 0;

/**
 * Constructor of Reader should read the filename, and select segments.
 */
Reader::Reader(const std::string &filename) : m_filename(filename) {
  utils::file2xml(filename, m_doc);
  std::string method = Config::Instance()->GetString("code-selection");
  if (method == "loop") {
    getLoopSegments();
  } else if (method == "annotation") {
    getAnnotationSegments();
  } else if (method == "divide") {
    getDivideSegments();
  } else {
    // assert(false && "segment selection method is not recognized: " && method.c_str());
    std::cerr<<"segment selection method is not recognized: " <<method<<"\n";
    assert(false);
  }
}

/**
 * Count line breaks in s before pattern.
 */
int count_line(const std::string &s, std::string pattern) {
  std::string sub = s.substr(0, s.find(pattern));
  return std::count(sub.begin(), sub.end(), '\n')+1;
}

TEST(reader_test_case, count_line) {
  std::string s = "hello\nworld\nhebi";
  int count = count_line(s, "hebi");
  EXPECT_EQ(count, 3);
}

/**
 * Use line numbers for a segment selection.
 */
Reader::Reader(const std::string &filename, std::vector<int> line_numbers)
  : m_filename(filename)
{
  utils::file2xml(filename, m_doc);
  std::vector<NodeKind> kinds;
  kinds.push_back(NK_DeclStmt);
  kinds.push_back(NK_ExprStmt);
  NodeList nodes = ast::find_nodes_on_lines(m_doc, kinds, line_numbers);
  for (Node n : nodes) {
    Segment seg;
    seg.PushBack(n);
    m_segments.push_back(seg);
  }
}


/*
 * For every segment:
 * - increase context
 * - process
 * - create builder
 * - create tester
 * - create analyzer
 */

void mylog(std::string s) {
  utils::append_file("./output.txt", s);
}
void mylog(int a) {
  utils::append_file("./output.txt", std::to_string(a));
}
void mylogln() {
  utils::append_file("./output.txt", "\n");
}

void
Reader::Read() {
  for (Segment &seg : m_segments) {
    std::cout <<"processing another segment .."  << "\n";
    for(;seg.IsValid();) {

      /*******************************
       ** Processing segment
       *******************************/
      std::cout <<"================"  << "\n";
      // std::cout <<utils::CYAN<<seg.GetText()  << utils::RESET << "\n";
      seg.ResolveInput();
      // seg.ResolveOutput();
      seg.ResolveSnippets();

      /** outputing input variables */
      VariableList vars = seg.GetInputVariables();
      // VariableList out_vars = seg.GetOutputVariables();
      std::cout <<"input vars: "<<vars.size()  << "\n";
      for (Variable v : vars) {
        std::cout <<"\t" << v.Name() << ":" << v.GetType().ToString()  << "\n";
      }


      /*******************************
       ** Outputing code
       *******************************/
      
      // std::cout <<out_vars.size()  << "\n";
      std::string main_text = seg.GetMain();
      std::string support = seg.GetSupport();
      std::string makefile = seg.GetMakefile();

      utils::print(seg.GetContextText(), utils::CK_Blue);
      // utils::print(main_text, utils::CK_Blue);
      // std::cout <<main_text  << "\n";
      
      // use tmp dir everytime
      char tmp_dir[] = "/tmp/helium-test-temp.XXXXXX";
      char *result = mkdtemp(tmp_dir);
      assert(result != NULL);
      std::string dir = tmp_dir;
      utils::write_file(dir+"/main.c", main_text);
      utils::write_file(dir+"/support.h", support);
      utils::write_file(dir + "/Makefile", makefile);

      std::cout << "code outputed to: "<<dir << " .." << "\n";
      // main_text.substr(main_text.find("@HeliumStmt"));
      // int seg_line = std::count(main_text.begin(), main_text.begin()+main_text.find("@HeliumStmt"), '\n');
      int seg_line = count_line(main_text, "@HeliumSegment")+1;
      int stmt_line = count_line(main_text, "@HeliumStmt")+1;
      if (stmt_line == seg_line+1) seg_line++;
      utils::print(seg_line, CK_Yellow);
      // getchar();

      /*******************************
       * debugging
       *******************************/

      // std::set<Snippet*> snippets = SnippetRegistry::Instance()->Resolve("ns_nameok");
      // std::cout <<"ns_nameok:"  << "\n";
      // std::cout <<snippets.size()  << "\n";

      
      /*******************************
       ** Compiling
       *******************************/
      std::string clean_cmd = "make clean -C " + dir;
      std::string cmd = "make -C " + dir;
      cmd += " 2>&1";
      utils::exec(clean_cmd.c_str(), NULL);
      int return_code;
      std::string error_msg = utils::exec(cmd.c_str(), &return_code);
      if (return_code == 0) {
        utils::print("success", utils::CK_Green);
      } else {
        utils::print("error", utils::CK_Red);
      }
      
      if (return_code==0) {
        int val_correct_failure=0;
        int val_wrong_failure=0;
        int val_success=0;
        int return_success = 0;
        int return_failure = 0;
        srand(time(0));

        /*******************************
         ** Running tests
         *******************************/
        std::string executable = dir+"/a.out";
        std::string test_dir = dir + "/test";
        utils::create_folder(test_dir);
        int test_number = Config::Instance()->GetInt("test-number");
        for (int i=0;i<test_number;i++) { // 10 tests each
          std::string input;
          for (Variable v : vars) {
            input += get_random_input(v.GetType());
          }
          std::cout <<"input:"  <<input<< "\n";
          std::string input_filename = test_dir + "/test-" + std::to_string(i) + "-input.txt";
          std::string valgrind_xml_filename = test_dir + "/test-" + std::to_string(i) + "-valgrind.xml";
          utils::write_file(input_filename, input);
          std::string run_cmd =
            "valgrind --xml=yes --xml-file="
            + valgrind_xml_filename
            + " " + executable + "< " + input_filename + " > /dev/null 2>&1";
          int status=0;
          // std::cout <<run_cmd  << "\n";
          utils::exec(run_cmd.c_str(), &status);
          if (status == 0) {
            // utils::print("run success", utils::CK_Cyan);
            return_success++;
          } else {
            // std::cout <<"run fail. Exit code: " << status  << "\n";
            return_failure++;
          }
          /*******************************
           ** parse valgrind
           *******************************/
          pugi::xml_document val_doc;
          // std::cout <<"segline:"<<seg_line  << "\n";
          val_doc.load_file(valgrind_xml_filename.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
          std::set<int> lines;
          for (auto error : val_doc.select_nodes("//error")) {
            Node error_node = error.node();
            std::string kind = error_node.child_value("kind");
            // std::cout <<kind  << "\n";
            if (kind != "InvalidRead" && kind != "InvalidWrite") continue;
            for (Node frame : error_node.child("stack").children("frame")) {
              std::string filename = frame.child_value("file");
              std::string line = frame.child_value("line");
              // std::cout <<line  << "\n";
              // utils::print(line, utils::CK_Purple);
              if (filename == "main.c") {
                lines.insert(atoi(line.c_str()));
                utils::print(kind+":"+line, utils::CK_Cyan);
              }
            }
          }
          if (lines.empty()) {
            val_success ++;
          } else if (lines.find(seg_line) != lines.end()) {
            val_correct_failure++;
          } else {
            val_wrong_failure++;
          }
          // utils::print(seg_line, utils::CK_Yellow);
        } // tests
        std::cout << utils::PURPLE <<"val_success: " << val_success  << "\n";
        std::cout <<"val_correct_failure: " << val_correct_failure  << "\n";
        std::cout <<"val_wrong_failure: " << val_wrong_failure << "\n";
        std::cout <<"return success: " << return_success  << "\n";
        std::cout <<"return fail: " << return_failure  << "\n";
        std::cout << utils::RESET<< "\n";
        mylog(val_success);
        mylog(",");
        mylog(val_correct_failure);
        mylog(",");
        mylog(val_wrong_failure);
        mylog(",");
        mylog(return_success);
        mylog(",");
        mylog(return_failure);
        mylog("\n");
        
      } // compile success

      // Builder builder(seg);
      // builder.Build();
      // builder.Compile();
      seg.IncreaseContext();
    }
    std::cout <<seg.GetInvalidReason()  << "\n";
  }
}

// void
// Reader::Read() {
//   // signal(SIGALRM, watch_dog);
//   // ualarm(Config::Instance()->GetInt("segment_timeout")*1000, 0);
//   for (auto it=m_spus.begin();it!=m_spus.end();it++) {
//     //    if (setjmp(jmpbuf) != 0) perror("setjmp");
//     // setjmp(jmpbuf);
//     // if (watch_dog_skip) {
//     //   watch_dog_skip = false;
//     //   continue;
//     // }
//     m_cur_seg_no ++;
//     if (m_skip_segment > m_cur_seg_no) {
//       continue;
//     }
//     // process the segment unit.
//     // do input resolve, output resovle, context search, support resolve
//     (*it).Process();
//     do {
//       // library call experiment
//       VariableList inv = (*it).GetInputVariables();
//       VariableList outv = (*it).GetOutputVariables();
      
//       Builder builder(*it);
//       builder.Build();
//       if (!(*it).CanContinue()) {
//         break;
//       }
//       builder.Compile();
//       // if (builder.Success() && Config::Instance()->WillRunTest()) {
//       //   std::shared_ptr<Tester> tester = std::make_shared<Tester>(builder.GetExecutable(), *it);
//       //   tester->Test();
//       //   if (tester->Success() && Config::Instance()->WillRunAnalyze()) {
//       //     std::shared_ptr<Analyzer> analyzer = std::make_shared<Analyzer>(tester->GetOutput());
//       //     analyzer->Analyze();
//       //   }
//       // }
//     } while ((*it).IncreaseContext());
//   }
// }

void Reader::getLoopSegments() {
  // pugi::xpath_query loop_query("//while|//for");
  // pugi::xpath_node_set loop_nodes = loop_query.evaluate_node_set(*m_doc);
  // // std::cout<<loop_nodes.size()<<std::endl;
  // if (!loop_nodes.empty()) {
  //   for (auto it=loop_nodes.begin();it!=loop_nodes.end();it++) {
  //     // every node is a loop segment!
  //     SPU su = std::make_shared<SegmentProcessUnit>(m_filename);
  //     su->AddNode(it->node());
  //     if (su->IsValid()) {
  //       m_spus.push_back(su);
  //     }
  //   }
  // }

}
/**
 * Get segemnt based on annotation in source code.
 * 1. @HeliumStmt
 * 2. @HeliumStart -- @HeliumStop TODO
 */
void Reader::getAnnotationSegments() {
  NodeList comment_nodes = find_nodes_containing_str(m_doc, NK_Comment, "@HeliumStmt");
  for (Node node : comment_nodes) {
    Segment seg;
    seg.PushBack(helium_next_sibling(node));
    m_segments.push_back(seg);
  }
  // pugi::xml_node root = m_doc->document_element();
  // pugi::xpath_node_set comment_nodes = root.select_nodes("//comment");
  // for (auto it=comment_nodes.begin();it!=comment_nodes.end();it++) {
  //   pugi::xml_node node = it->node();
  //   std::string comment_text = DomUtil::GetTextContent(node);
  //   if (comment_text.find("@HeliumStart") != std::string::npos) {
  //     SPU su = std::make_shared<SegmentProcessUnit>(m_filename);
  //     su->AddNode(node);
  //     while (node.next_sibling()) {
  //       node = node.next_sibling();
  //       su->AddNode(node);
  //       if (node.type() == pugi::node_element && strcmp(node.name(), "comment") == 0) {
  //         std::string comment_text = DomUtil::GetTextContent(node);
  //         if (comment_text.find("@HeliumStop") != std::string::npos) {
  //           break;
  //         }
  //       }
  //     }
  //     if (su->IsValid()) {
  //       m_spus.push_back(su);
  //     }
  //   }
  // }
}

/*
 * for every function, divide code by comments, start of loop, start of branch condition.
 * combine these blocks
 
 treat comment as ast block
for every block, if it is simple statement, combine into a NodeList.
If it is a block of interest(Loop, Condition), treat it singlely as a NodeList.
 */
void
Reader::getDivideSegments() {
  NodeList functions = find_nodes(m_doc, NK_Function);
  for (Node function : functions) {
    Node block = function_get_block(function);
    getDivideRecursive(block_get_nodes(block));
  }
}

static bool is_leaf_node(Node node) {
  switch(ast::kind(node)) {
  case NK_ExprStmt:
  case NK_Return:
  case NK_Continue:
  case NK_Break:
  case NK_Define:
  case NK_IfnDef:
  case NK_IfDef:
  case NK_DefElse:
  case NK_EndIf:
  case NK_DeclStmt: return true;
  case NK_If:
  case NK_Comment:
  case NK_Do:
  case NK_While:
  case NK_Switch:
  case NK_For: return false;
  default:
    std::cerr<<ast::kind_to_name(ast::kind(node)) << " is not recoganized or handled.\n";
    assert(false);
    return true;
  }
}

/**
 * Get body node list of block.
 * This block is not <block>, but AST level.
 * Such as <if>
 * If can have two blocks, <then><block> and <else><block>.
 * The algorithm is to enumerate the type, and get the one we want.
 */
std::vector<NodeList> get_block_bodies(Node node) {
  std::vector<NodeList> result;
  switch(ast::kind(node)) {
  case NK_If: {
    Node then_block = ast::if_get_then_block(node);
    // TODO should move this into the uppr function, if all blocks here have a <block> tag.
    result.push_back(block_get_nodes(then_block));
    Node else_block = ast::if_get_else_block(node);
    result.push_back(block_get_nodes(else_block));
    break;
  }
  case NK_For: {
    Node block = ast::for_get_block(node);
    result.push_back(block_get_nodes(block));
    break;
  }
  case NK_Switch: {
    NodeList cases = ast::switch_get_cases(node);
    for (Node case_node : cases) {
      result.push_back(ast::case_get_nodes(case_node));
    }
    break;
  }
  case NK_Do: {
    Node block = ast::do_get_block(node);
    result.push_back(block_get_nodes(block));
    break;
  }
  case NK_While: {
    Node block = ast::while_get_block(node);
    result.push_back(block_get_nodes(block));
    break;
  }
  default: ;
  }
  return result;
}


void Reader::getDivideRecursive(NodeList nodes) {
  std::vector<NodeList> seg_cands;
  NodeList block_nodes;
  NodeList tmp_group;

  // step 1
  for (Node node : nodes) {
    if (is_leaf_node(node)) {
      tmp_group.push_back(node);
    } else {
      if (!tmp_group.empty()) seg_cands.push_back(tmp_group);
      tmp_group.clear();
      if (ast::kind(node) != NK_Comment) {
        NodeList l = {node}; // need this in document algorithm?
        seg_cands.push_back(l);
        block_nodes.push_back(node);
      }
    }
  }
  if (!tmp_group.empty()) {
    seg_cands.push_back(tmp_group);
    tmp_group.clear();
  }

  // constructing the final segments from segment candidates
  for (size_t i=0;i<seg_cands.size();i++) {
    for (size_t j=i;j<seg_cands.size();j++) {
      Segment seg;
      for (size_t k=i;k<=j;k++) {
        seg.PushBack(seg_cands[k]);
      }
      m_segments.push_back(seg);
    }
  }

  // process block
  for (Node block : block_nodes) {
    std::vector<NodeList> bodies = get_block_bodies(block);
    for (NodeList body : bodies) {
      getDivideRecursive(body);
    }
  }
  
}




void Reader::PrintSegments() {
  for (Segment &seg : m_segments) {
    std::cout <<"==========="  << "\n";
    std::cout << seg.GetText() << "\n";
  }
}
  


/*******************************
 ** Deprecated code
 *******************************/

/*
 * True if variables contains type "name", "number" types
 */
// bool
// has_variable(std::set<std::shared_ptr<Variable> > variables, std::string name, int number) {
//   std::shared_ptr<Type> type = TypeFactory(name).CreateType();
//   if (number <= 0) return true;
//   for (auto it=variables.begin();it!=variables.end();it++) {
//     std::shared_ptr<Type> vtype = (*it)->GetType();
//     if (type->GetName() == vtype->GetName()) {
//         // && type->GetDimension() == vtype->GetDimension()
//         // && type->GetPointerLevel() == vtype->GetPointerLevel()) {
//       number--;
//       if (number <=0) return true;
//     }
//   }
//   return false;
// }

// bool
// has_variables(std::set<std::shared_ptr<Variable> > variables, std::string);



// static bool watch_dog_skip = false;

// static jmp_buf jmpbuf; // jumbuf for long jump
// static bool skip_file = false;

// void watch_dog(int sig) {
//   global_error_number++;
//   if (global_error_number>100) exit(1);
//   // file_error_number++;
//   // if (file_error_number>30) {
//   //   skip_file = true;
//   // }
//   watch_dog_skip = true;
//   ualarm(Config::Instance()->GetSegmentTimeout()*1000, 0);
//   longjmp(jmpbuf, 1); // jump back to previous stack
// }

// std::string
// get_match_library_name(std::set<std::shared_ptr<Variable> > inv) {
//   std::vector< std::set<std::string> > libraries;
//   libraries.push_back({"char", "int"});
//   for (auto it=libraries.begin();it!=libraries.end();it++) {
//     for (auto jt=it->begin();jt!=it->end();jt++) {
//       // if (!has_variable(inv, *jt, 1))
//     }
//   }
//   if (has_variable(inv, "char*", 1)
//       && has_variable(inv, "int", 1)) {
//     std::cout<<"FOUND segment that has the same input variable as library call"<<std::endl;
//     // std::cout<< "\033[1;30m " << (*it)->GetSegment()->GetText()<< " \033[0m" <<std::endl;
//     // OK, the input variable matches
//   } 
// }

