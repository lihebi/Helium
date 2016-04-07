#ifndef SNIPPET_DB_H
#define SNIPPET_DB_H
#include "common.h"

namespace snippetdb {
  void create_snippet_db(std::string tagfile, std::string output_folder);
  std::set<int> get_all_dependence(std::set<int> snippet_ids);
  std::set<int> get_dependence(int id);
  std::vector<int> sort_snippets(std::set<int> snippets);
}

#endif /* SNIPPET-DB_H */
