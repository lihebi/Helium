#include <cstring>
#include "Reader.hpp"
#include "util/ThreadUtil.hpp"
#include "Builder.hpp"
#include "Tester.hpp"
#include "Analyzer.hpp"
#include "util/SrcmlUtil.hpp"
#include "util/DomUtil.hpp"
#include "Logger.hpp"
#include "type/TypeFactory.hpp"
#include <signal.h>
#include <setjmp.h>
#include "util/FileUtil.hpp"
#include "Global.hpp"

int Reader::m_skip_segment = -1;
int Reader::m_cur_seg_no = 0;


/*
 * 1. srcml the file into m_doc
 * 2. call getSegments()
 */
Reader::Reader(const std::string &filename)
: m_filename(filename) {
  Logger::Instance()->LogTrace("[Reader][Reader]\n");
  std::cout<<m_filename<<std::endl;
  m_doc = std::make_shared<pugi::xml_document>();
  SrcmlUtil::File2XML(m_filename, *m_doc);
  getSegments();
  Logger::Instance()->LogTrace("[Reader] Total segment in this file: "
  + std::to_string(m_seg_units.size()) + "\n");
  std::cout<<"total seg: " << m_seg_units.size()<<std::endl;
  if (m_seg_units.size() > 0 && Config::Instance()->WillInteractReadSegment()) {
    Logger::Instance()->LogTrace("[Reader::Reader] Done reading segment\n");
    getchar();
  }
  m_skip_segment = Config::Instance()->GetSkipSegment();
}
Reader::~Reader() {}

/*
 * True if variables contains type "name", "number" types
 */
bool
has_variable(std::set<std::shared_ptr<Variable> > variables, std::string name, int number) {
  std::shared_ptr<Type> type = TypeFactory(name).CreateType();
  if (number <= 0) return true;
  for (auto it=variables.begin();it!=variables.end();it++) {
    std::shared_ptr<Type> vtype = (*it)->GetType();
    if (type->GetName() == vtype->GetName()
        && type->GetDimension() == vtype->GetDimension()
        && type->GetPointerLevel() == vtype->GetPointerLevel()) {
      number--;
      if (number <=0) return true;
    }
  }
  return false;
}

// bool
// has_variables(std::set<std::shared_ptr<Variable> > variables, std::string);


static bool watch_dog_skip = false;

static jmp_buf jmpbuf; // jumbuf for long jump

void watch_dog(int sig) {
  Logger::Instance()->Log("Segment Timeout\n");
  global_error_number++;
  if (global_error_number>100) exit(1);
  watch_dog_skip = true;
  ualarm(Config::Instance()->GetSegmentTimeout()*1000, 0);
  longjmp(jmpbuf, 1); // jump back to previous stack
}

std::string
get_match_library_name(std::set<std::shared_ptr<Variable> > inv) {
  std::vector< std::set<std::string> > libraries;
  libraries.push_back({"char", "int"});
  for (auto it=libraries.begin();it!=libraries.end();it++) {
    for (auto jt=it->begin();jt!=it->end();jt++) {
      // if (!has_variable(inv, *jt, 1))
    }
  }
  if (has_variable(inv, "char*", 1)
      && has_variable(inv, "int", 1)) {
    std::cout<<"FOUND segment that has the same input variable as library call"<<std::endl;
    // std::cout<< "\033[1;30m " << (*it)->GetSegment()->GetText()<< " \033[0m" <<std::endl;
    // OK, the input variable matches
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
void
Reader::Read() {
  signal(SIGALRM, watch_dog);
  Logger::Instance()->LogTrace("[Reader][Read]\n");
  ualarm(Config::Instance()->GetSegmentTimeout()*1000, 0);
  for (auto it=m_seg_units.begin();it!=m_seg_units.end();it++) {
    //    if (setjmp(jmpbuf) != 0) perror("setjmp");
    setjmp(jmpbuf);
    if (watch_dog_skip) {
      watch_dog_skip = false;
      continue;
    }
    m_cur_seg_no ++;
    if (m_skip_segment > m_cur_seg_no) {
      continue;
    }
    Logger::Instance()->Log(
                            "begin segment:  NO: " + std::to_string(m_cur_seg_no)
                            + "\n"
                            );
    // process the segment unit.
    // do input resolve, output resovle, context search, support resolve
    (*it)->Process();
    do {
      // library call experiment
      std::set<std::shared_ptr<Variable> > inv = (*it)->GetInputVariables();
      std::set<std::shared_ptr<Variable> > outv = (*it)->GetOutputVariables();

      // output inv, outv
      std::cout<<"\tinput:";
      for (auto it=inv.begin();it!=inv.end();it++) {
        std::cout<<(*it)->GetType()->GetName()<<" ";
      }
      std::cout<<"\n\toutput:";
      for (auto it=outv.begin();it!=outv.end();it++) {
        std::cout<<"\t"<<(*it)->GetType()->GetName()<<std::endl;
      }
      std::cout<<std::endl;
      
      // TODO how to compare variable?
      // prototype from string.h
      // (char*, int) (char*, char*)
      // prototype from ctype.h
      // (int, int)
      if (has_variable(inv, "char*", 1)
          && has_variable(inv, "int", 1)) {
        std::cout<<"FOUND segment that has the same input variable as library call"<<std::endl;
        std::cout<< "\033[1;30m " << (*it)->GetSegment()->GetText()<< " \033[0m" <<std::endl;


        // std::string filename = "/Users/hebi/benchmark/char_int.txt";
        std::string filename = "./char_int.txt";
        FileUtil::Append(filename, "\n================\n");
        FileUtil::Append(filename, "\tSegNO: "+std::to_string(m_cur_seg_no)+"\n");
        FileUtil::Append(filename, "\tFilename: "+m_filename+"\n");
        FileUtil::Append(filename, (*it)->GetSegment()->GetText());
        // OK, the input variable matches
      }
      continue;

      
      std::shared_ptr<Builder> builder = std::make_shared<Builder>(*it);
      builder->Build();
      if (!(*it)->CanContinue()) {
        Logger::Instance()->LogWarning("[Reader::Read] segment cannot continue");
        break;
      }
      builder->Compile();
      if (builder->Success() && Config::Instance()->WillRunTest()) {
        std::shared_ptr<Tester> tester = std::make_shared<Tester>(builder->GetExecutable(), *it);
        tester->Test();
        if (tester->Success() && Config::Instance()->WillRunAnalyze()) {
          std::shared_ptr<Analyzer> analyzer = std::make_shared<Analyzer>(tester->GetOutput());
          analyzer->Analyze();
        }
      }
    } while ((*it)->IncreaseContext());
  }
}

void Reader::getSegments() {
  if (Config::Instance()->GetCodeSelectionMethod() == "loop") {
    getLoopSegments();
  } else if (Config::Instance()->GetCodeSelectionMethod() == "annotation") {
    getAnnotationSegments();
  } else if (Config::Instance()->GetCodeSelectionMethod() == "divide") {
    getDivideSegments();
  }
}

void Reader::getLoopSegments() {
  pugi::xpath_query loop_query("//while|//for");
  pugi::xpath_node_set loop_nodes = loop_query.evaluate_node_set(*m_doc);
  // std::cout<<loop_nodes.size()<<std::endl;
  if (!loop_nodes.empty()) {
    for (auto it=loop_nodes.begin();it!=loop_nodes.end();it++) {
      // every node is a loop segment!
      std::shared_ptr<SegmentProcessUnit> su = std::make_shared<SegmentProcessUnit>(m_filename);
      su->AddNode(it->node());
      if (su->IsValid()) {
        m_seg_units.push_back(su);
      }
    }
  }

}
void Reader::getAnnotationSegments() {
  Logger::Instance()->LogTrace("[Reader::getAnnotationSegments]\n");
  pugi::xml_node root = m_doc->document_element();
  pugi::xpath_node_set comment_nodes = root.select_nodes("//comment");
  for (auto it=comment_nodes.begin();it!=comment_nodes.end();it++) {
    pugi::xml_node node = it->node();
    std::string comment_text = DomUtil::GetTextContent(node);
    if (comment_text.find("@HeliumStart") != std::string::npos) {
      std::shared_ptr<SegmentProcessUnit> su = std::make_shared<SegmentProcessUnit>(m_filename);
      su->AddNode(node);
      while (node.next_sibling()) {
        node = node.next_sibling();
        su->AddNode(node);
        if (node.type() == pugi::node_element && strcmp(node.name(), "comment") == 0) {
          std::string comment_text = DomUtil::GetTextContent(node);
          if (comment_text.find("@HeliumStop") != std::string::npos) {
            break;
          }
        }
      }
      if (su->IsValid()) {
        m_seg_units.push_back(su);
      }
    }
  }
}

bool
is_comment(pugi::xml_node n) {
  if (!n) return false;
  if (strcmp(n.name(), "comment") == 0) return true;
  return false;
}

bool
is_branch(pugi::xml_node n) {
  if (!n) return false;
  if (strcmp(n.name(), "if") == 0 || strcmp(n.name(), "switch") == 0) return true;
  return false;
}

bool
is_loop(pugi::xml_node n) {
  if (!n) return false;
  if (strcmp(n.name(), "for") == 0
      || strcmp(n.name(), "while") == 0
      || strcmp(n.name(), "do") == 0) {
    return true;
  }
  return false;
}

/*
 * for every function, divide code by comments, start of loop, start of branch condition.
 * combine these blocks
 */
void
Reader::getDivideSegments() {
  Logger::Instance()->LogTrace("[Reader::getDivideSegments]\n");
  pugi::xml_node root = m_doc->document_element();
  pugi::xpath_node_set function_nodes = root.select_nodes("//function");
  for (auto it=function_nodes.begin();it!=function_nodes.end();it++) {
    // for each function
    // divide into blocks (vector of vector of nodes)
    std::vector< std::vector<pugi::xml_node> > blocks;
    pugi::xml_node node = it->node();
    pugi::xml_node block_node = node.child("block");
    if (block_node) {
      pugi::xml_node n = block_node.first_child();
      n = n.next_sibling(); // the first child of block is {, so we start from the second
      std::vector<pugi::xml_node> block;
      while (n && n != block_node.last_child()) { // should also remove the case of the last child (})
        if (is_comment(n) || is_branch(n) || is_loop(n)) {
          // block can be empty: 1) at the beginning 2) consequtive comments
          if (!block.empty()) {
            blocks.push_back(block);
            block.clear();
          }
        }
        // push if not comment
        if (!is_comment(n)) {
          block.push_back(n);
        }
        n = n.next_sibling();
      }
    }
    // combine blocks into segments
    for (int i=0;i<blocks.size();i++) {
      for (int j=i+1;j<blocks.size();j++) {
        // create SPU and insert
        std::shared_ptr<SegmentProcessUnit> su = std::make_shared<SegmentProcessUnit>(m_filename);
        for (int k=i;k<j;k++) {
          su->AddNodes(blocks[k]);
        }
        if (su->IsValid()) {
          m_seg_units.push_back(su);
        }
      }
    }
  }
}
