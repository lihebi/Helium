/**
 * Persist and reload from SQLite database.
 * Or, may just provide some API for manage the database, e.g. look up a snippet by its kind, add a snippet, etc.
 * But for now, just persist it.
 */
#include <gtest/gtest.h>
#include <readtags.h>
#include <sqlite3.h>
#include "utils/utils.h"
#include "snippet.h"
#include "resolver.h"

#include "snippet_db.h"
#include "parser/xml_doc_reader.h"

#include "utils/dot.h"

using namespace utils;

SnippetDB *SnippetDB::m_instance = NULL;


/*******************************
 * PART 1: create DB
 *******************************/

/**
 * 1. Insert snippet into "snippet" table
 * 2. Insert the signatures into "signature" table
 * 3. Write the code to "code/<id>.txt"
 * @return snippet ID
 */
int SnippetDB::insertSnippet(Snippet *snippet) {
  assert(snippet);
  assert(snippet->IsValid());
  static int snippet_id = -1;
  static int signature_id = 0;
  // the first snippet_id is 0!
  snippet_id++;

  // char buf[100] = "insert into snippet values (10, 'fd', 3);";
  // char buf[100] = "select * from snippet;";
  char buf[BUFSIZ];
  /**
   * Insert into snippet table
   */
  const char* format;
  // | ID | filename        | line number |
  format = "insert into snippet values (%d, '%s', %d);";
  snprintf(buf, BUFSIZ, format,
          snippet_id,
          snippet->GetFileName().c_str(),
          snippet->GetLineNumber());
  // std::cout << buf  << "\n";
  char *errmsg = NULL;
  int rc = sqlite3_exec(m_db, buf, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    // FIXME LOG? ASSERT?
    fprintf(stderr, "SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
    assert(false);
  }
  /**
   * Insert into signature table
   */

  format = "insert into signature values (%d, '%s', '%c', %d);";
  // | ID | keyword | kind        | snippet_id |
  SnippetSignature sig = snippet->GetSignature();
  for (auto m : sig) {
    std::string key = m.first;
    for (SnippetKind k : m.second) {
      char c = snippet_kind_to_char(k);
      snprintf(buf, BUFSIZ, format,
               signature_id,
               key.c_str(),
               c, snippet_id);
      // std::cout << buf  << "\n";
      int rc = sqlite3_exec(m_db, buf, NULL, NULL, &errmsg);
      signature_id++;
      if (rc != SQLITE_OK) {
        // FIXME LOG? ASSERT?
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
        assert(false);
      }
    }
  }
  return snippet_id;
}

void SnippetDB::InsertHeaderDep(std::string from, std::string to) {
  static int header_dep_id = 0;
  const char *format = "insert into header_dep values (%d, '%s', '%s');";
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, format,
           header_dep_id, from.c_str(), to.c_str()
           );
  char *errmsg = NULL;
  int rc = sqlite3_exec(m_db, buf, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
    assert(false);
  }
  header_dep_id++;
}
std::vector<std::pair<std::string, std::string> > SnippetDB::GetHeaderDep() {
  std::vector<std::pair<std::string, std::string> > ret;
  ret = queryStrStr("select from_file, to_file from header_dep");
  return ret;
}


/**
 * Call Graph Construction
 */
std::map<std::string, std::set<std::string> > SnippetDB::constructCG(std::map<std::string, int> &all_functions) {
  std::map<std::string, std::set<std::string> > cg;
  for (auto m : all_functions) {
    // get code for m.first
    std::string func_name = m.first;
    int snippet_id = m.second;
    std::string code = GetCode(snippet_id);
    // ast::XMLDoc *doc = XMLDocReader::Instance()->ReadString(code);
    std::vector<std::string> calls = XMLDocReader::QueryCode(code, "//call/name");
    for (std::string call : calls) {
      if (all_functions.count(call) == 1) {
        cg[func_name].insert(call);
      }
    }
  }
  return cg;
}

/**
 * Create call graph.
 *
 * For all the functions in the database:
 * 1. get the code
 * 2. use srcml to parse
 * 3. get the called functions, record: <called function name, caller>
 * 4. for all the records:
 *    query database for the called fucntion name
 *    add entry for each such record
 */
void SnippetDB::createCG() {
  sqlite3 *db = m_db;
  std::cout << "create_callgraph"  << "\n";
  assert(db);
  /**
   * Process all functions
   */
  std::map<std::string, int> all_functions = queryFunctions();
  std::map<std::string, std::set<std::string> > cg = constructCG(all_functions);
  /**
   * Insert into db
   */
  const char* format = "insert into callgraph values (%d, %d, %d);";
  char buf[BUFSIZ];
  int id=0;
  for (auto m : cg) {
    std::string from_func = m.first;
    assert(all_functions.count(from_func) == 1);
    int from_id = all_functions[from_func];
    for (const std::string &to_func : m.second) {
      assert(all_functions.count(to_func) == 1);
      int to_id = all_functions[to_func];
      snprintf(buf, BUFSIZ, format, id, from_id, to_id);
      id++;
      char *errmsg = NULL;
      int rc = sqlite3_exec(db, buf, NULL, NULL, &errmsg);
      if (rc != SQLITE_OK) {
        // FIXME LOG? ASSERT?
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
        assert(false);
      }
    }
  }
}

/**
 * Traverse through the snippet table and build the dependence table.
 */
void SnippetDB::createDep() {
  // select * from snippet order by ID
  // for every snippet, get the code
  // 1. loop through 1 .. ID_MAX
  // 2. resolve the code => depnedent ids
  // 3. query for the ids, => snippet IDs
  // select snippet_ID from signature where key=<Keyword>
  // 4. add dependence
  std::cout << "creating dependence .."  << "\n";
  int dependence_id = 0;
  int rc = 0;
  std::vector<int> snippet_ids = queryInt("select ID from snippet");
  for (int id : snippet_ids) {
    std::cout << '.' << std::flush;
    std::string code_file = m_db_folder + "/code/"+std::to_string(id) + ".txt";
    std::string code = utils::read_file(code_file);
    // FIXME NOW This is very bad!
    // The code may contains comments!
    std::set<std::string> names = extract_id_to_resolve(code);
    for (std::string name : names) {
      /**
       * prepare stmt, execute, and construct dependence
       */
      std::set<int> deps;
      const char *format = "select snippet_id from signature where keyword='%s';";
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, format, name.c_str());
      sqlite3_stmt *stmt;
      rc = sqlite3_prepare_v2(m_db, buf, -1, &stmt, NULL);
      assert(rc == SQLITE_OK);
      while (true) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
          // data row
          assert(sqlite3_column_count(stmt) == 1);
          int tmp_id = sqlite3_column_int(stmt, 0);
          deps.insert(tmp_id);
        } else if (rc == SQLITE_DONE) {
          break;
        } else {
          assert(false);
        }
      }
      sqlite3_finalize(stmt);
      // remove itself
      deps.erase(id);
      /**
       * Insert dependence | ID | from_snippet_id | to_snippet_id |
       */
      format = "insert into dependence values (%d, %d, %d);";
      for (int dep : deps) {
        snprintf(buf, BUFSIZ, format, dependence_id, id, dep);
        dependence_id++;
        char *errmsg = NULL;
        rc = sqlite3_exec(m_db, buf, NULL, NULL, &errmsg);
        if (rc != SQLITE_OK) {
          // FIXME LOG? ASSERT?
          fprintf(stderr, "SQL error: %s\n", errmsg);
          sqlite3_free(errmsg);
          assert(false);
        }
      }
    }
  }
  std::cout  << "\n";
  std::cout << "total dependence: " << dependence_id  << "\n";
}

void SnippetDB::createTable() {
  const char *sql = R"prefix(
  create table snippet (
    ID INT, filename VARCHAR(500), linum INT,
    PRIMARY KEY (ID)
    );
  create table signature (
    ID INT, keyword VARCHAR(100), kind VARCHAR(1), snippet_id int,
    PRIMARY KEY (ID),
    FOREIGN KEY (snippet_id) REFERENCES snippet(ID)
    );
  create table dependence (
    ID int, from_snippet_id int, to_snippet_id int,
    primary key (ID),
    foreign key (from_snippet_id) references snippet(ID),
    foreign key (to_snippet_id) references snippet(ID)
    );
  create table callgraph (
    ID int, from_snippet_id int, to_snippet_id int,
    primary key (ID),
    foreign key (from_snippet_id) references snippet(ID),
    foreign key (to_snippet_id) references snippet(ID)
    );
  create table header_dep (
    ID int, from_file VARCHAR(100), to_file VARCHAR(100),
    primary key(ID)
    );
)prefix";
  char *errmsg = NULL;
  assert(m_db);
  int rc = sqlite3_exec(m_db, sql, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    // FIXME LOG? ASSERT?
    fprintf(stderr, "SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
    assert(false);
  }
}


/**
 * According to, and only to, the tagfile, create a database of snippets.
 * The output will be:
 * output_folder/index.db
 * output_folder/code/<number>.txt
 */
void SnippetDB::Create(std::string tagfile, std::string output_folder) {
  std::cerr << "creating snippet db ..."  << "\n";
  if (utils::exists(output_folder)) {
    utils::remove_folder(output_folder);
  }
  utils::create_folder(output_folder);
  utils::create_folder(output_folder+"/code");
  std::string db_file = output_folder+"/index.db";
  sqlite3_open(db_file.c_str(), &m_db);
  assert(m_db);
  createTable();
  m_db_folder = output_folder;
  /**
   * Opening tag file
   */
  std::cerr << "opening tag file ..."  << "\n";
  tagFile *tag = NULL;
  tagFileInfo *info = (tagFileInfo*)malloc(sizeof(tagFileInfo));
  tag = tagsOpen(tagfile.c_str(), info);
  if (info->status.opened != true) {
    assert(false);
  }
  free(info);

  /**
   * Iterating tag file
   */
  std::cout << "iterating .."  << "\n";
  tagEntry *entry = (tagEntry*)malloc(sizeof(tagEntry));
  tagResult res = tagsFirst(tag, entry);
  int snippet_id = 0;
  std::vector<Snippet*> all_snippets;
  while (res == TagSuccess) {
    /**
     * Inserting database
     */
    std::cout << '.' << std::flush;
    CtagsEntry ctags_entry(entry);
    // std::cout << "creating for snippet: " << ctags_entry.GetName();
    Snippet *snippet = new Snippet(ctags_entry);
    if (snippet != NULL) {
      if (snippet->IsValid()) {
        // std::cout << ": *********************************!";
        /**
         * Since we are iterating the tag file, for the enumerator, we will add them many times.
         * Apparently this is not desired.
         * However, should I handle the duplication at the time of returning the snippets? At the time of query?
         */
        snippet_id = insertSnippet(snippet);
        std::string code_file = output_folder + "/code/" + std::to_string(snippet_id) + ".txt";
        utils::write_file(code_file, snippet->GetCode());
      }
      all_snippets.push_back(snippet);
      // delete snippet;
    }
    // std::cout  << "\n";
    res = tagsNext(tag, entry);
  }
  std::cout  << "\n";
  std::cout << "total snippet: " << snippet_id + 1  << "\n";
  int define_ct = 0;
  int func_ct = 0;
  int struct_ct = 0;
  int enum_ct = 0;
  int enum_mem_ct = 0;
  int typedef_ct = 0;
  int var_ct = 0;
  int other_ct = 0;
  for (Snippet *s : all_snippets) {
    SnippetKind k = s->MainKind();
    switch (k) {
    case SK_Define: define_ct++; break;
    case SK_Function: func_ct++; break;
    case SK_Structure: struct_ct++; break;
    case SK_Enum: enum_ct++; break;
    case SK_EnumMember: enum_mem_ct++; break;
    case SK_Typedef: typedef_ct++; break;
    case SK_Variable: var_ct++; break;
    default:
      // std::cout << k  << "\n";
      other_ct++; break;
    }
  }
  std::cout << "define snippet: " << define_ct  << "\n";
  std::cout << "function snippet: " << func_ct  << "\n";
  std::cout << "struct snippet: " << struct_ct  << "\n";
  std::cout << "enum snippet: " << enum_ct  << "\n";
  std::cout << "enum member snippet: " << enum_mem_ct  << "\n";
  std::cout << "typedef snippet: " << typedef_ct  << "\n";
  std::cout << "variable snippet: " << var_ct  << "\n";
  std::cout << "other snippet: " << other_ct  << "\n";
  for (Snippet *s : all_snippets) {
    delete s;
  }
  // TODO print out snippet details, like how many functions, defines, structures, etc.
  createDep();
  createCG();
  sqlite3_close_v2(m_db);
}

/********************************
 * Debugging
 *******************************/

/**
 * Print out CG as dot graph
 */
void SnippetDB::PrintCG() {
  const char *query = "select from_snippet_id, to_snippet_id from callgraph;";
  std::vector<std::pair<int, int> > res = queryIntInt(query);
  DotGraph dotgraph;
  for (auto m : res) {
    int from_id = m.first;
    int to_id = m.second;
    std::string from_name = GetMeta(from_id).GetKey();
    std::string to_name = GetMeta(to_id).GetKey();
    // dotgraph.AddNodeIfNotExist(from_name, from_name);
    // dotgraph.AddNodeIfNotExist(to_name, to_name);
    dotgraph.AddNode(from_name, from_name);
    dotgraph.AddNode(to_name, to_name);
    dotgraph.AddEdge(from_name, to_name);
  }
  std::string dot = dotgraph.dump();
  std::cout << dot  << "\n";
}



/*******************************
 * PART 2: Use DB
 *******************************/

/********************************
 * CallGraph
 *******************************/
void SnippetDB::Load(std::string folder) {
  m_db_folder = folder;
  std::string db_file = folder + "/index.db";
  sqlite3_open(db_file.c_str(), &m_db);

  loadCG();
}

void SnippetDB::loadCG() {
  const char *query = "select from_snippet_id, to_snippet_id from callgraph;";
  std::vector<std::pair<int, int> > res = queryIntInt(query);
  for (auto m : res) {
    int from_id = m.first;
    int to_id = m.second;
    std::string from_name = GetMeta(from_id).GetKey();
    std::string to_name = GetMeta(to_id).GetKey();
    m_cg[from_name].insert(to_name);
    m_reverse_cg[to_name].insert(from_name);
  }
}

/**
 * return all the callers on the call graph
 */
std::set<std::string> SnippetDB::QueryCallers(std::string func_name) {
  std::set<std::string> ret;
  if (m_reverse_cg.count(func_name) == 0) return ret;
  ret = m_reverse_cg[func_name];
  return ret;
}
/**
 * Return one caller (the first one in the set)
 */
std::string SnippetDB::QueryCaller(std::string func_name) {
  std::set<std::string> callers = QueryCallers(func_name);
  if (callers.empty()) return "";
  else return *callers.begin();
}

std::set<int> SnippetDB::LookUp(SnippetKind kind) {
  std::set<int> ret;
  assert(m_db);
  char k = snippet_kind_to_char(kind);
  std::string query = "select snippet_id from signature where kind='";
  query += k;
  query += "';";
  std::vector<int> v = queryInt(query.c_str());
  ret.insert(v.begin(), v.end());
  return ret;
}


/**
 * Look up the key in the database.
 * Only return those whose kind is in kinds.
 * Must call load_db before, and the db must be successfully loaded.
 * @param [in] kinds if empty, select all kinds
 * @return snippet_ids
 */
std::set<int> SnippetDB::LookUp(std::string key, std::set<SnippetKind> kinds) {
  std::set<int> ret;
  assert(m_db);
  const char *format = "select snippet_id,kind from signature where keyword='%s';";
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, format, key.c_str());
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(SnippetDB::Instance()->m_db, buf, -1, &stmt, NULL);
  assert(rc == SQLITE_OK && "Try use absolute path of --snippet-db-folder option");
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 2);
      int tmp_id = sqlite3_column_int(stmt, 0);
      const unsigned char *s = sqlite3_column_text(stmt, 1);
      assert(s);
      char kind = *s;
      // so this kind only apply to the main kind!
      if (kinds.empty() || kinds.count(char_to_snippet_kind(kind)) == 1) {
        ret.insert(tmp_id);
      }
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}

/**
 * Look up a bunch of keywords.
 */
std::set<int> SnippetDB::LookUp(std::set<std::string> keys, std::set<SnippetKind> kinds) {
  std::set<int> ret;
  for (std::string key : keys) {
    std::set<int> tmp = LookUp(key, kinds);
    ret.insert(tmp.begin(), tmp.end());
  }
  return ret;
}



SnippetMeta SnippetDB::GetMeta(int snippet_id) {
  if (m_snippet_cache.count(snippet_id) == 1) {
    return m_snippet_cache[snippet_id];
  }
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, "select filename, linum from snippet where ID=%d;", snippet_id);
  std::vector<std::pair<std::string, int> > res = queryStrInt(buf);
  assert(res.size() == 1);
  std::string filename = res[0].first;
  int linum = res[0].second;
  SnippetMeta meta(filename, linum);
  snprintf(buf, BUFSIZ, "select keyword,kind from signature where snippet_id=%d;", snippet_id);
  std::vector<std::pair<std::string, char> > res2 = queryStrChar(buf);
  for (std::pair<std::string, char> sig : res2) {
    meta.AddSignature(sig.first, char_to_snippet_kind(sig.second));
  }
  m_snippet_cache[snippet_id] = meta;
  return meta;
}

std::string SnippetDB::GetCode(int snippet_id) {
  if (m_snippet_code_cache.count(snippet_id) == 1) {
    return m_snippet_code_cache[snippet_id];
  }
  std::string code_file = m_db_folder+"/code/"+std::to_string(snippet_id) + ".txt";
  assert(utils::exists(code_file));
  m_snippet_code_cache[snippet_id] = utils::read_file(code_file);
  return m_snippet_code_cache[snippet_id];
}

std::set<int> SnippetDB::GetDep(int id) {
  std::set<int> ret;
  assert(m_db);
  const char *format = "select to_snippet_id from dependence where from_snippet_id=%d;";
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, format, id);
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(m_db, buf, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 1);
      int tmp_id = sqlite3_column_int(stmt, 0);
      ret.insert(tmp_id);
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}

/*******************************
 * queries
 *******************************/
/**
 * The query is something like:
 * select col1 from table where ...
 * It should not select two, otherwise assert fail
 */
std::vector<int> SnippetDB::queryInt(const char *query) {
  std::vector<int> ret;
  // assert(SnippetDB::Instance()->db != NULL);
  assert(m_db);
  sqlite3_stmt *stmt;
  int rc;
  // rc = sqlite3_prepare_v2(SnippetDB::Instance()->db, query, -1, &stmt, NULL);
  rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 1);
      int res = sqlite3_column_int(stmt, 0);
      ret.push_back(res);
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}

std::vector<std::string> SnippetDB::queryStr(const char *query) {
  std::vector<std::string> ret;
  // assert(SnippetDB::Instance()->db != NULL);
  assert(m_db);
  sqlite3_stmt *stmt;
  int rc;
  // rc = sqlite3_prepare_v2(SnippetDB::Instance()->db, query, -1, &stmt, NULL);
  rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 1);
      // FIXME can I cast from uchar to char like this?
      const char *res = (char*)sqlite3_column_text(stmt, 0);
      assert(res);
      std::string res_str(res);
      ret.push_back(res_str);
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}

std::vector<std::pair<int, int> > SnippetDB::queryIntInt(const char *query) {
  std::vector<std::pair<int, int> > ret;
  // assert(SnippetDB::Instance()->db != NULL);
  assert(m_db);
  sqlite3_stmt *stmt;
  int rc;
  // rc = sqlite3_prepare_v2(SnippetDB::Instance()->db, query, -1, &stmt, NULL);
  rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 2);
      // FIXME can I cast from uchar to char like this?
      int first = sqlite3_column_int(stmt, 0);
      int second = sqlite3_column_int(stmt, 1);
      ret.push_back(std::make_pair(first, second));
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}


std::vector<std::pair<int, std::string> > SnippetDB::queryIntStr(const char *query) {
  std::vector<std::pair<int, std::string> > ret;
  // assert(SnippetDB::Instance()->db != NULL);
  assert(m_db);
  sqlite3_stmt *stmt;
  int rc;
  // rc = sqlite3_prepare_v2(SnippetDB::Instance()->db, query, -1, &stmt, NULL);
  rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 2);
      // FIXME can I cast from uchar to char like this?
      int first = sqlite3_column_int(stmt, 0);
      const char *second = (char*)sqlite3_column_text(stmt, 1);
      assert(second);
      std::string second_str(second);
      ret.push_back(std::make_pair(first, second_str));
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}

std::vector<std::pair<std::string, int> > SnippetDB::queryStrInt(const char *query) {
  std::vector<std::pair<std::string, int> > ret;
  // assert(SnippetDB::Instance()->db != NULL);
  assert(m_db);
  sqlite3_stmt *stmt;
  int rc;
  // rc = sqlite3_prepare_v2(SnippetDB::Instance()->db, query, -1, &stmt, NULL);
  rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 2);
      // FIXME can I cast from uchar to char like this?
      const char *first = (char*)sqlite3_column_text(stmt, 0);
      assert(first);
      std::string first_str(first);
      int second = sqlite3_column_int(stmt, 1);
      ret.push_back(std::make_pair(first_str, second));
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}


std::vector<std::pair<std::string, char> > SnippetDB::queryStrChar(const char* query) {
  std::vector<std::pair<std::string, char> > ret;
  // assert(SnippetDB::Instance()->db != NULL);
  assert(m_db);
  sqlite3_stmt *stmt;
  int rc;
  // rc = sqlite3_prepare_v2(SnippetDB::Instance()->db, query, -1, &stmt, NULL);
  rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 2);
      std::string first = (char*)sqlite3_column_text(stmt, 0);
      const unsigned char *s = sqlite3_column_text(stmt, 1);
      assert(s);
      char second = *s;
      ret.push_back({first, second});
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}

std::vector<std::pair<std::string, std::string> > SnippetDB::queryStrStr(const char *query) {
  std::vector<std::pair<std::string, std::string> > ret;
  // assert(SnippetDB::Instance()->db != NULL);
  assert(m_db);
  sqlite3_stmt *stmt;
  int rc;
  // rc = sqlite3_prepare_v2(SnippetDB::Instance()->db, query, -1, &stmt, NULL);
  rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 2);
      const char *first = (char*)sqlite3_column_text(stmt, 0);
      assert(first);
      std::string first_str(first);
      const char *second = (char*)sqlite3_column_text(stmt, 1);
      assert(second);
      std::string second_str(second);
      ret.push_back(std::make_pair(first_str, second_str));
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}


std::map<std::string, int> SnippetDB::queryFunctions() {
  const char *query = "select keyword, snippet_id from signature where kind='f'";
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  // all functions, functionname -> snippet id
  // FIXME would there be same function name? Yes, static functions.
  // But I will not consider it ^_^
  std::map<std::string, int> all_functions;
  while(true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      assert(sqlite3_column_count(stmt) == 2);
      int snippet_id = sqlite3_column_int(stmt, 1);
      const char *name = (char*)sqlite3_column_text(stmt, 0);
      assert(name);
      std::string name_str(name);
      all_functions[name] = snippet_id;
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return all_functions;
}

/*******************************
 * get dependences
 *******************************/

std::set<int> SnippetDB::GetAllDep(std::set<int> snippet_ids) {
  std::set<int> ret;
  std::set<int> worklist;
  worklist.insert(snippet_ids.begin(), snippet_ids.end());
  while (!worklist.empty()) {
    int id = *worklist.begin();
    worklist.erase(id);
    ret.insert(id);
    std::set<int> ids = GetDep(id);

    // DEBUG
    // if (snippet_ids.count(id)==1) {
    //   std::cout << id   << ": ";
    //   for (int id : ids) {
    //     std::cout << id  << ",";
    //   }
    //   std::cout   << "\n";
    // }
    
    for (int id : ids) {
      if (ret.count(id) == 0) {
        worklist.insert(id);
      }
    }
  }
  return ret;
}

/**
 * Compare the signature. If same, only retain one.
 */
std::set<int> SnippetDB::RemoveDup(std::set<int> snippet_ids) {
  std::set<std::map<std::string, std::set<SnippetKind> > > seen_sigs;
  std::set<int> ret;
  for (int id : snippet_ids) {
    SnippetMeta meta = GetMeta(id);
    if (seen_sigs.count(meta.signature) != 1) {
      seen_sigs.insert(meta.signature);
      ret.insert(id);
    }
  }
  return ret;
}

std::vector<int> SnippetDB::SortSnippets(std::set<int> snippets) {
  std::vector<int> ret;
  /**
   * 1. get the file names for all snippets
   * 2. sort each file by line number
   * 2. put all c files at the end
   * 2. use HeaderSorter to sort
   * 3. map back to snippet ids
   */
  std::map<std::string, std::vector<int> > file_to_snippet_id_map;
  // step 1
  for (int id : snippets) {
    char buf[BUFSIZ];
    snprintf(buf, BUFSIZ, "select filename from snippet where ID=%d", id);
    std::vector<std::string> tmp = queryStr(buf);
    assert(tmp.size() == 1);
    file_to_snippet_id_map[tmp[0]].push_back(id);
  }
  /**
   * sort each file by line number
   */
  for (auto it=file_to_snippet_id_map.begin();it!=file_to_snippet_id_map.end();it++) {
    std::sort(it->second.begin(), it->second.end(),
              [](int id1, int id2)-> bool {
                return SnippetDB::Instance()->GetMeta(id1).linum < SnippetDB::Instance()->GetMeta(id2).linum;
              });
  }
  
  std::set<std::string> sources;
  std::set<std::string> headers;
  for (auto m : file_to_snippet_id_map) {
    std::string filename = m.first;
    if (filename.back() == 'h') {
      headers.insert(filename);
    } else {
      sources.insert(filename);
    }
  }
  std::vector<std::string> sorted_headers = HeaderResolver::Instance()->Sort(headers);
  for (std::string header : sorted_headers) {
    ret.insert(ret.end(),
               file_to_snippet_id_map[header].begin(),
               file_to_snippet_id_map[header].end()
               );
  }
  for (std::string source : sources) {
    ret.insert(ret.end(),
               file_to_snippet_id_map[source].begin(),
               file_to_snippet_id_map[source].end()
               );
  }
  return ret;
}


/**
 * Populate the "header_dependence" table
 */
// void create_header_dependence(sqlite3 *db, std::string benchmark_folder) {
//   assert(db);
//   /**
//    * 1. get all =.h= files in the benchmark folder
//    * 2. get their canonical names and simple names
//    * 3. insert into "header" table
//    * 3. get all includes of them => simple name
//    * 4. query to get the id, then 
//    */
//   std::vector<std::string> headers;
//   get_files_by_extension(folder, headers, "h");
//   for (auto it=headers.begin();it!=headers.end();it++) {
//     std::string filename = *it;
//     // get only the last component(i.e. filename) in the file path
//     filename = filename.substr(filename.rfind("/")+1);
//     std::ifstream is(*it);
//     if (is.is_open()) {
//       std::string line;
//       while(std::getline(is, line)) {
//         // process line
//         boost::smatch match;
//         if (boost::regex_search(line, match, include_reg)) {
//           std::string new_file = match[1];
//           // the filename part of including
//           if (new_file.find("/") != std::string::npos) {
//             new_file = new_file.substr(new_file.rfind("/")+1);
//           }
//           // add the dependence
//           // FIXME if the include is in the middle of the header file,
//           // The dependence may change.
//           addDependence(filename, new_file);
//         }
//       }
//       is.close();
//     }
//   }
  
// }
