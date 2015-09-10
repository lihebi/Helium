#include "Segment.hpp"
#include <iostream>
#include "util/DomUtil.hpp"

Segment::Segment () {}
Segment::~Segment () {}

void Segment::PushBack(pugi::xml_node node) {
  m_nodes.push_back(node);
}
void Segment::PushFront(pugi::xml_node node) {
  m_nodes.insert(m_nodes.begin(), node);
}
void Segment::Clear() {
  m_nodes.clear();
}

std::vector<pugi::xml_node> Segment::GetNodes() {
  return m_nodes;
}

void Segment::Print() {
  std::cout<<"=======Segment======="<<std::endl;
  for (auto it=m_nodes.begin();it!=m_nodes.end();it++) {
    // it->print(std::cout);
    std::cout<<DomUtil::GetTextContent(*it)<<std::endl;
  }
}
