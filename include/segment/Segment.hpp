#ifndef __SEGMENT_HPP__
#define __SEGMENT_HPP__

#include <pugixml/pugixml.hpp>
#include <vector>

class Segment {
public:
  Segment ();
  virtual ~Segment ();
  void PushBack(pugi::xml_node node);
  void PushFront(pugi::xml_node node);
  void Clear();
  void Print();
  std::vector<pugi::xml_node> GetNodes() const;
  pugi::xml_node GetFirstNode() const;
  std::string GetText();
private:
  std::vector<pugi::xml_node> m_nodes;
};

#endif
