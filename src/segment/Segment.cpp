#include "segment/Segment.hpp"
#include "util/DomUtil.hpp"
#include <algorithm>

#include <iostream>

Segment::Segment () {}
Segment::~Segment () {}

void Segment::PushBack(pugi::xml_node node) {
  m_nodes.push_back(node);
  std::string text = GetText();
  m_loc = std::count(text.begin(), text.end(), '\n');
}
void Segment::PushFront(pugi::xml_node node) {
  m_nodes.insert(m_nodes.begin(), node);
  std::string text = GetText();
  m_loc = std::count(text.begin(), text.end(), '\n');
}
void Segment::Clear() {
  m_nodes.clear();
  m_loc = 0;
}

/*
 * Return line number of first node.
 * 0 if no pos::line found for all node.
 */
int
Segment::GetLineNumber() const {
  pugi::xml_node node;
  for (auto it=m_nodes.begin();it!=m_nodes.end();it++) {
    try {
      node = it->select_node("//*[@pos::line]").node();
    } catch(pugi::xpath_exception) {
      // TODO
    }
    if (node) return atoi(node.attribute("pos::line").value());
  }
  return 0;
}

std::vector<pugi::xml_node> Segment::GetNodes() const {
  return m_nodes;
}

pugi::xml_node
Segment::GetFirstNode() const {
  if (m_nodes.empty()) {
    // this should be a node_null
    return pugi::xml_node();
  } else {
    return m_nodes[0];
  }
}

void Segment::Print() {
  std::cout<<"=======Segment======="<<std::endl;
  for (auto it=m_nodes.begin();it!=m_nodes.end();it++) {
    // it->print(std::cout);
    std::cout<<DomUtil::GetTextContent(*it)<<std::endl;
  }
}

std::string
Segment::GetText() {
  std::string s;
  for (auto it=m_nodes.begin();it!=m_nodes.end();it++) {
    s += DomUtil::GetTextContent(*it) + '\n';
  }
  return s;
}

bool
Segment::HasNode(pugi::xml_node node) const {
  for (auto it=m_nodes.begin();it!=m_nodes.end();it++) {
    if (DomUtil::lub(*it, node) == *it) return true;
  }
  return false;
}
