#ifndef __SEGMENT_HPP__
#define __SEGMENT_HPP__

#include <pugixml.hpp>
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
  std::string GetTextExceptComment();
  int GetLineNumber() const;
  int GetLOC() const {return m_loc;}
  bool HasNode(pugi::xml_node node) const;
private:
  std::vector<pugi::xml_node> m_nodes;
  int m_loc = 0;
};

#endif
