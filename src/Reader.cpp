#include <Reader.hpp>
#include "util/ThreadUtil.hpp"
#include "Builder.hpp"
#include "Tester.hpp"
#include "Analyzer.hpp"
#include "util/SrcmlUtil.hpp"
#include "util/DomUtil.hpp"
#include "Logger.hpp"

int Reader::m_skip_segment = -1;
Reader::Reader(const std::string &filename)
: m_filename(filename) {
  std::cout<<"[Reader][Constructor]"<<filename<<std::endl;
  m_doc = std::make_shared<pugi::xml_document>();
  SrcmlUtil::File2XML(m_filename, *m_doc);
  getSegments();
  std::cout<<"[Reader] Total segment in this file: "<<m_seg_units.size()<<std::endl;
  if (m_seg_units.size() > 0 && Config::Instance()->WillInteractReadSegment()) {
    std::cout << "[Reader::Reader] Done reading segment" << std::endl;
    getchar();
  }
  if (m_skip_segment == -1) {
    m_skip_segment = Config::Instance()->GetSkipSegment();
  }
}
Reader::~Reader() {}

void
Reader::Read() {
  std::cout<<"[Reader][Read]"<<std::endl;
  for (auto it=m_seg_units.begin();it!=m_seg_units.end();it++) {
    if (m_skip_segment > 0) {
      m_skip_segment--;
      Logger::Instance()->Log("skip this segment: " + m_filename);
      continue;
    } else {
      Logger::Instance()->Log("a new segment" + m_filename);
    }
    // process the segment unit.
    // do input resolve, output resovle, context search, support resolve
    (*it)->Process();
    do {
      std::shared_ptr<Builder> builder = std::make_shared<Builder>(*it);
      builder->Build();
      builder->Compile();
      if (builder->Success()) {
        std::shared_ptr<Tester> tester = std::make_shared<Tester>(builder->GetExecutable(), *it);
        if (tester->Success()) {
          std::shared_ptr<Analyzer> analyzer = std::make_shared<Analyzer>(tester->GetOutput());
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
  std::cout << "[Reader::getAnnotationSegments]" << std::endl;
  pugi::xml_node root = m_doc->document_element();
  pugi::xpath_node_set comment_nodes = root.select_nodes("//comment");
  for (auto it=comment_nodes.begin();it!=comment_nodes.end();it++) {
    pugi::xml_node node = it->node();
    std::string comment_text = DomUtil::GetTextContent(node);
    if (comment_text.find("@HeliumStart") != -1) {
      std::shared_ptr<SegmentProcessUnit> su = std::make_shared<SegmentProcessUnit>(m_filename);
      su->AddNode(node);
      while (node.next_sibling()) {
        node = node.next_sibling();
        su->AddNode(node);
        if (node.type() == pugi::node_element && strcmp(node.name(), "comment") == 0) {
          std::string comment_text = DomUtil::GetTextContent(node);
          if (comment_text.find("@HeliumStop") != -1) {
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
