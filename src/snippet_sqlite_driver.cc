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

using namespace utils;
TEST(SQLiteTestCase, DriverTest) {
  sqlite3 *db;
  sqlite3_open("./data/test.db", &db);
}

// static int callback(void *NotUsed, int argc, char **argv, char **azColName){
//    int i;
//    for(i=0; i<argc; i++){
//       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//    }
//    printf("\n");
//    return 0;
// }

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
void create_snippet_db(std::string tagfile, std::string output_folder) {
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

