#ifndef DOT_H
#define DOT_H

#include <vector>
#include <string>
#include "common.h"


/**
 * Dot Graph Helper
 */

class DotNode {
public:
  DotNode(std::string id, std::string label) : m_id(id), m_label(label) {
    m_label.erase(std::remove(m_label.begin(), m_label.end(), '"'), m_label.end());
  }
  ~DotNode() {}
  std::string GetID() const {return m_id;}
  std::string GetLabel() const {return m_label;}
  void SetText(std::string text) {
    m_text = text;
  }
  void AddText(std::string text) {
    m_text += " " + text;
  }
  std::string GetText() {return m_text;}
  std::string GetCode();
private:
  std::string m_id;
  std::string m_label;
  std::string m_text;
};

class DotEdge {
public:
  DotEdge(DotNode *from, DotNode *to, std::string label="") : m_from(from), m_to(to), m_label(label) {}
  ~DotEdge() {}
  std::string GetCode();
private:
  DotNode *m_from;
  DotNode *m_to;
  std::string m_label;
};

class DotGraph {
public:
  DotGraph() {}
  ~DotGraph() {
    for (auto m : m_nodes) {
      if (m.second) {
        delete m.second;
      }
    }
    for (DotEdge *edge : m_edges) {
      delete edge;
    }
  }
  void AddNode(std::string id, std::string label);
  void AddEdge(std::string id_from, std::string id_to, std::string label="");
  void AddText(std::string id, std::string text);
  std::string dump();
private:
  std::map<std::string, DotNode*> m_nodes;
  std::vector<DotEdge*> m_edges;
};


#endif /* DOT_H */
