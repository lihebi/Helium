/**
 * Persist and reload from SQLite database.
 * Or, may just provide some API for manage the database, e.g. look up a snippet by its kind, add a snippet, etc.
 * But for now, just persist it.
 */
#include <gtest/gtest.h>
#include <readtags.h>
#include <sqlite3.h>
#include "utils.h"
#include "snippet.h"
#include "resolver.h"

#include "snippet_db.h"

using namespace utils;

/**
 * 1. Insert snippet into "snippet" table
 * 2. Insert the signatures into "signature" table
 * 3. Write the code to "code/<id>.txt"
 * @return snippet ID
 */
int insert_snippet(sqlite3 *db, Snippet *snippet) {
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
  int rc = sqlite3_exec(db, buf, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    // FIXME LOG? ASSERT?
    fprintf(stderr, "SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
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
      int rc = sqlite3_exec(db, buf, NULL, NULL, &errmsg);
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

/**
 * Traverse through the snippet table and build the dependence table.
 */
void create_dependence(sqlite3 *db, std::string code_folder, int max_snippet_id) {
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
  for (int id=0;id<=max_snippet_id;id++) {
    std::cout << '.';
    std::string code_file = code_folder+"/"+std::to_string(id) + ".txt";
    std::string code = utils::read_file(code_file);
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
      rc = sqlite3_prepare_v2(db, buf, -1, &stmt, NULL);
      assert(rc == SQLITE_OK);
      while (true) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
          // data row
          assert(sqlite3_column_count(stmt) == 1);
          int tmp_id = sqlite3_column_int(stmt, 0);
          assert(tmp_id >=0 && tmp_id <= max_snippet_id);
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
        rc = sqlite3_exec(db, buf, NULL, NULL, &errmsg);
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

void create_table(sqlite3 *db) {
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
  create table header_dependence (
    ID int, from_header_id int, to_header_id int,
    primary key (ID),
    foreign key (from_header_id) references header(ID),
    foreign key (to_header_id) references header(ID),
    );
)prefix";
  char *errmsg = NULL;
  int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    // FIXME LOG? ASSERT?
    fprintf(stderr, "SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
  }
}


/**
 * According to, and only to, the tagfile, create a database of snippets.
 * The output will be:
 * output_folder/index.db
 * output_folder/code/<number>.txt
 */
void snippetdb::create_snippet_db(std::string tagfile, std::string output_folder) {
  std::cout << "creating snippet db ..."  << "\n";
  if (utils::exists(output_folder)) {
    utils::remove_folder(output_folder);
  }
  utils::create_folder(output_folder);
  utils::create_folder(output_folder+"/code");
  std::string db_file = output_folder+"/index.db";
  sqlite3 *db;
  sqlite3_open(db_file.c_str(), &db);
  create_table(db);

  /**
   * Opening tag file
   */
  std::cout << "opening tag file ..."  << "\n";
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
  while (res == TagSuccess) {
    /**
     * Inserting database
     */
    CtagsEntry ctags_entry(entry);
    Snippet *snippet = new Snippet(ctags_entry);
    if (snippet != NULL) {
      if (snippet->IsValid()) {
        snippet_id = insert_snippet(db, snippet);
        std::string code_file = output_folder + "/code/" + std::to_string(snippet_id) + ".txt";
        utils::write_file(code_file, snippet->GetCode());
      }
      delete snippet;
    }
    res = tagsNext(tag, entry);
  }
  std::cout << "total snippet: " << snippet_id + 1  << "\n";
  create_dependence(db, output_folder+"/code", snippet_id);
  sqlite3_close_v2(db);
}

static std::string db_folder;
static sqlite3 *db_instance = NULL;

void load_db(std::string folder) {
  db_folder = folder;
  std::string db_file = db_folder + "/index.db";
  sqlite3_open(db_file.c_str(), &db_instance);
}

/**
 * Look up the key in the database.
 * Only return those whose kind is in kinds.
 * Must call load_db before, and the db must be successfully loaded.
 * @return snippet_ids
 */
std::set<int> look_up_snippet(std::string key, std::set<SnippetKind> kinds) {
  std::set<int> ret;
  assert(db_instance != NULL);
  const char *format = "select snippet_id,kind from signature where keyword='%s';";
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, format, key.c_str());
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(db_instance, buf, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 2);
      int tmp_id = sqlite3_column_int(stmt, 0);
      const unsigned char *s = sqlite3_column_text(stmt, 1);
      assert(s);
      char kind = *s;
      if (kinds.count(char_to_snippet_kind(kind)) == 1) {
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

std::set<int> snippetdb::get_dependence(int id) {
  std::set<int> ret;
  assert(db_instance != NULL);
  const char *format = "select to_snippet_id from dependence where from_snippet_id=%d;";
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, format, id);
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(db_instance, buf, -1, &stmt, NULL);
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

/**
 * The query is something like:
 * select col1 from table where ...
 * It should not select two, otherwise assert fail
 */
std::vector<int> query_first_int(const char *query) {
  std::vector<int> ret;
  assert(db_instance != NULL);
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(db_instance, query, -1, &stmt, NULL);
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

std::vector<std::string> query_first_string(char *query) {
  std::vector<std::string> ret;
  assert(db_instance != NULL);
  sqlite3_stmt *stmt;
  int rc;
  rc = sqlite3_prepare_v2(db_instance, query, -1, &stmt, NULL);
  assert(rc == SQLITE_OK);
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      assert(sqlite3_column_count(stmt) == 1);
      // FIXME can I cast from uchar to char like this?
      const char *res = (char*)sqlite3_column_text(stmt, 1);
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

std::set<int> snippetdb::get_all_dependence(std::set<int> snippet_ids) {
  std::set<int> ret;
  std::set<int> worklist;
  worklist.insert(snippet_ids.begin(), snippet_ids.end());
  while (!worklist.empty()) {
    int id = *worklist.begin();
    worklist.erase(id);
    ret.insert(id);
    std::set<int> ids = get_dependence(id);
    for (int id : ids) {
      if (ret.count(id) == 0) {
        worklist.insert(id);
      }
    }
  }
  return ret;
}

std::vector<int> snippetdb::sort_snippets(std::set<int> snippets) {
  std::vector<int> ret;
  /**
   * 1. get the file names for all snippets
   * 2. use HeaderSorter to sort
   * 3. map back to snippet ids
   */
  std::map<std::string, std::set<int> > file_to_snippet_id_map;
  // step 1
  for (int id : snippets) {
    char buf[BUFSIZ];
    snprintf(buf, BUFSIZ, "select filename from snippet where ID=%d", id);
    std::vector<std::string> tmp = query_first_string(buf);
    assert(tmp.size() == 1);
    file_to_snippet_id_map[tmp[0]].insert(id);
  }
  std::set<std::string> filenames;
  for (auto m : file_to_snippet_id_map) {
    filenames.insert(m.first);
  }
  std::vector<std::string> sorted_filenames = HeaderSorter::Instance()->Sort(filenames);
  for (std::string filename : sorted_filenames) {
    ret.insert(ret.begin(),
               file_to_snippet_id_map[filename].begin(),
               file_to_snippet_id_map[filename].end()
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
