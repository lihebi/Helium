#include "SegmentUnit.hpp"
#include "util/DomUtil.hpp"

SegmentUnit::SegmentUnit(const Config &config)
: m_segment(std::make_shared<Segment>()),
  m_context(std::make_shared<Segment>()),
  m_config(config) {
  ;
}
SegmentUnit::~SegmentUnit() {}

void SegmentUnit::SetSegment(const Segment &s) {
  m_segment = std::make_shared<Segment>(s);
}
void SegmentUnit::AddSegmentNode(pugi::xml_node node) {
  m_segment->PushBack(node);
}
void SegmentUnit::Process() {
  std::cout<<"[SegmentUnit] Processing Segment Unit"<<std::endl;
  contextSearch();
}

void SegmentUnit::contextSearch() {
  if (m_config.context_search == "linear") {
    int max_linear_search_value = m_config.max_linear_search_value;
    for (int i=0;i<max_linear_search_value;i++) {
      linearSearch(i);
    }
  }
}

bool SegmentUnit::increaseContext(int value) {
  // increase context
  pugi::xml_node first_node = m_context->GetNodes()[0];
  pugi::xml_node tmp = DomUtil::GetPreviousASTElement(first_node);
  if (tmp) {
    // sibling nodes
    m_context->PushFront(tmp);
  } else {
    tmp = DomUtil::GetParentASTElement(first_node);
    if (!tmp) return false;
    if (strcmp(tmp.name(), "function") == 0) {
      // interprocedure
      m_functions.push_back(tmp);
      tmp = DomUtil::GetFunctionCall(tmp);
      m_context->Clear();
      m_context->PushBack(tmp);
    } else {
      // parent node
      m_context->Clear();
      m_context->PushBack(tmp);
    }
  }
  return true;
}

void SegmentUnit::linearSearch(int linear_value) {
  int i=0;
  *m_context = *m_segment;
  while(i<linear_value) {
    i++;
    std::cout<<"[SegmentUnit] linear search: "<<i<<std::endl;
    increaseContext(1);
    m_context->Print();
    // update m_inv
    resolveInput();
    // update m_outv
    resolveOutput();
    // add support
    resolveSupport();
  }
}

void SegmentUnit::resolveInput() {}
void SegmentUnit::resolveOutput() {}
void SegmentUnit::resolveSupport() {}
