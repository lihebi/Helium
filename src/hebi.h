#ifndef HEBI_H
#define HEBI_H
#include "workflow/reader.h"
#include "parser/cfg.h"

void hebi(std::string filename, POISpec poi);
void process(ASTNode *node);

class Query {
public:
  Query();
  ~Query();
  Query(const Query &q);
  ASTNode* New();
  void Merge(Query *q);
  void Add(ASTNode *node);
private:
};


std::set<Query*> find_mergable_query(ASTNode *node, Query *orig_query);
std::vector<Query*> select(Query *query);


#endif /* HEBI_H */
