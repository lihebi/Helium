#ifndef HEBI_H
#define HEBI_H
#include "workflow/reader.h"
#include "parser/cfg.h"

void hebi(std::string filename, POISpec poi);
void process(ASTNode *node);

class Query {
public:
  Query() {}
  ~Query() {}
  Query(const Query &q) {
    m_nodes = q.m_nodes;
    m_new = q.m_new;
  }
  Query(ASTNode *astnode);
  Query(CFGNode *cfgnode) {
    // TODO ensure this is not already in m_nodes, otherwise the m_new will not be ideal
    m_nodes.insert(cfgnode);
    m_new = cfgnode;
  }
  void Merge(Query *q) {
    std::set<CFGNode*> nodes = q->GetNodes();
    m_nodes.insert(nodes.begin(), nodes.end());
  }
  void Add(CFGNode *node) {
    assert(node);
    m_nodes.insert(node);
    m_new = node;
  }
  CFGNode* New() {
    return m_new;
  }
  std::set<CFGNode*> GetNodes() {
    return m_nodes;
  }
  bool ContainNode(CFGNode *node) {
    if (m_nodes.count(node) == 1) return true;
    else return false;
  }

  void Visualize(bool open=false);
private:
  std::set<CFGNode*> m_nodes;
  CFGNode *m_new = NULL;
};


std::set<Query*> find_mergable_query(CFGNode *node, Query *orig_query);
std::vector<Query*> select(Query *query);


#endif /* HEBI_H */
