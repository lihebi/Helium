#ifndef __HELIUM_H__
#define __HELIUM_H__

#include "common.h"
#include "reader.h"
#include "query.h"

#include "failure_point.h"
#include "point_of_interest.h"

class Helium {
public:
  // Helium(FailurePoint *fp);
  Helium(PointOfInterest *poi);
  ~Helium() {}
private:
  // void init(ASTNode *node);
  void process();
  std::vector<Query*> select(Query *query);
  std::set<Query*> find_mergable_query(CFGNode *node, Query *orig_query);
  // std::string derive_pre_cond(std::vector<std::string> invs, std::vector<std::string> trans);
  // bool pre_entry_point(std::string pre);
  // std::string merge_failure_condition(std::vector<std::string> invs);


  std::deque<Query*> m_worklist;
  std::map<CFGNode*, std::set<Query*> > m_waiting_quries;
  std::map<Query*, std::set<Query*> > m_propagating_queries;


  std::string m_failure_condition;
  // int countFunction();
};

std::set<Query*> find_mergable_query(CFGNode *node, Query *orig_query);
std::vector<Query*> select(Query *query);

std::vector<Variable> get_input_variables(std::set<CFGNode*> nodes);


#endif
