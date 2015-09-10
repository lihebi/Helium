#include <Reader.hpp>
#include "util/ThreadUtil.hpp"
#include "Builder.hpp"
#include "Tester.hpp"
#include "Analyzer.hpp"

Reader::Reader(const std::string &filename, const Config &config)
: m_config(config), m_filename(filename) {
  std::cout<<"[Reader] Constructor"<<std::endl;
  std::string cmd("src2srcml ");
  cmd += m_filename;
  std::string output = ThreadUtil::Exec(cmd.c_str());
  m_doc = std::make_shared<pugi::xml_document>();
  m_doc->load_string(output.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  getSegments();
  std::cout<<"[Reader] Total segment in this file: "<<m_seg_units.size()<<std::endl;
  for (auto it=m_seg_units.begin();it!=m_seg_units.end();it++) {
    // process the segment unit.
    // do input resolve, output resovle, context search, support resolve
    it->Process();
    // create builder
    std::shared_ptr<Builder> builder = std::make_shared<Builder>(*it, m_config);
    if (builder->Success()) {
      std::shared_ptr<Tester> tester = std::make_shared<Tester>(builder->GetExecutable(), *it, m_config);
      if (tester->Success()) {
        std::shared_ptr<Analyzer> analyzer = std::make_shared<Analyzer>(tester->GetOutput());
      }
    }
  }
}
Reader::~Reader() {}

void Reader::getSegments() {
  if (m_config.code_selection == "loop") {
    getLoopSegments();
  } else if (m_config.code_selection == "annotation") {
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
      SegmentUnit su(m_config);
      su.AddSegmentNode(it->node());
      m_seg_units.push_back(su);
    }
  }

}
void Reader::getAnnotationSegments() {
  ;
}
