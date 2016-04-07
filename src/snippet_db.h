#ifndef SNIPPET_DB_H
#define SNIPPET_DB_H
#include "common.h"
#include "snippet.h"
#include <sqlite3.h>
namespace snippetdb {

  // filename, linenumber, (keyword, kind)s
  class SnippetMeta {
  public:
    SnippetMeta() {}
    SnippetMeta(std::string f, int l) : filename(f), linum(l) {}
    void AddSignature(std::string key, SnippetKind k) {
      signature[key].insert(k);
    }
    bool HasKind(SnippetKind k) {
      for (auto sig : signature) {
        if (sig.second.count(k) == 1) return true;
      }
      return false;
    }
    /**
     * Get one key.
     */
    std::string GetKey() {
      assert(!signature.empty());
      return signature.begin()->first;
    }
    std::string filename;
    int linum=0;
    std::map<std::string, std::set<SnippetKind> > signature;
  };

  class SnippetDB {
  public:
    static SnippetDB *Instance() {
      if (!m_instance) {
        m_instance = new SnippetDB();
      }
      return m_instance;
    }
    std::string db_folder;
    sqlite3 *db;
    std::map<int, snippetdb::SnippetMeta> snippet_cache;
    std::map<int, std::string> snippet_code_cache;
  private:
    SnippetDB() {}
    static SnippetDB *m_instance;
  };
  void create_snippet_db(std::string tagfile, std::string output_folder);
  void load_db(std::string folder);
  std::set<int> get_all_dependence(std::set<int> snippet_ids);
  std::set<int> get_dependence(int id);
  std::vector<int> sort_snippets(std::set<int> snippets);
  std::set<int> look_up_snippet(std::string key, std::set<SnippetKind> kinds={});
  std::set<int> look_up_snippet(std::set<std::string> keys, std::set<SnippetKind> kinds={});
  std::vector<int> query_int(const char *query);
  std::vector<std::string> query_str(char *query);
  std::vector<std::pair<int, std::string> > query_int_str(char *query);
  std::vector<std::pair<std::string, int> > query_str_int(char *query);
  std::vector<std::pair<std::string, char> > query_str_char(char* query);
  SnippetMeta get_meta(int snippet_id);
  std::string get_code(int snippet_id);
}

#endif /* SNIPPET-DB_H */
