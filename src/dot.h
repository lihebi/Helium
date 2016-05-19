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
  DotNode(std::string id, std::string label) : m_id(id), m_label(label) {}
  ~DotNode() {}
  std::string GetID() const {return m_id;}
  std::string GetLabel() const {return m_label;}
private:
  std::string m_id;
  std::string m_label;
};

class DotGraph {
public:
  DotGraph() {}
  ~DotGraph() {
    for (auto m : m_id_m) {
      delete m.second;
    }
  }
  void AddNode(std::string id, std::string label) {
    assert(m_id_m.count(id) == 0);
    DotNode *node = new DotNode(id, label);
    m_id_m[id] = node;
  }
  void AddNodeIfNotExist(std::string id, std::string label) {
    if (m_id_m.count(id) == 1) return;
    AddNode(id, label);
  }
  DotNode* GetNode(std::string id) {
    assert(m_id_m.count(id) == 1);
    return m_id_m[id];
  }
  void AddEdge(DotNode* from, DotNode* to) {
    assert(from);
    assert(to);
    m_edge_m[from].insert(to);
  }
  void AddEdge(std::string id_from, std::string id_to) {
    assert(m_id_m.count(id_from) == 1);
    assert(m_id_m.count(id_to) == 1);
    m_edge_m[m_id_m[id_from]].insert(m_id_m[id_to]);
  }
  std::string dump() {
    std::string ret;
    ret += "digraph {\n";
    for (auto m : m_id_m) {
      DotNode* node = m.second;
      ret += node->GetID() + "[label=\"" + node->GetLabel() + "\"];\n";
    }
    for (auto m : m_edge_m) {
      DotNode* from = m.first;
      std::set<DotNode*> to = m.second;
      for (DotNode* t : to) {
        ret += from->GetID() + " -> " + t->GetID() + ";\n";
      }
    }
    ret += "}\n";
    return ret;
  }
private:
  std::map<std::string, DotNode*> m_id_m;
  std::map<DotNode*, std::set<DotNode*> > m_edge_m;
};


#endif /* DOT_H */
