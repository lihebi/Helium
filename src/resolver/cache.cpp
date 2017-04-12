#include "helium/resolver/cache.h"

#include "helium/utils/fs_utils.h"
#include "helium/utils/string_utils.h"
#include "helium/utils/thread_utils.h"
#include "helium/resolver/clangSnippet.h"
#include "helium/resolver/snippet.h"
#include "helium/resolver/snippet_db.h"

#include <iostream>
#include <vector>
using namespace std;


void create_src(fs::path target, fs::path target_cache_dir, fs::path target_sel_dir) {
  fs::path src = target_cache_dir / "src";
  if (fs::exists(src)) fs::remove_all(src);
  fs::create_directories(src);

  // fs::create_directories(target_cache_dir / "sel");
  fs::create_directories(target_sel_dir);
  
  // copy only source files. Keep directory structure
  fs::recursive_directory_iterator it(target), eod;
  BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      if (p.extension() == ".c" || p.extension() == ".h") {
        fs::path to = src / fs::relative(p, target);
        fs::create_directories(to.parent_path());
        // std::cout << "From: " << p << "\n";
        // std::cout << "Copying to " << to.string() << "\n";
        // this can also fail!!
        // fs::copy_file  will fail if file is found
        // 
        // even if I removed src folder at the beginning of this
        // procedure, it still happens because:
        // fs::relative_path will follow symbolic link
        if (fs::exists(to)) fs::remove(to);
        fs::copy_file(p, to);
      } else if (p.extension() == ".sel") {
        // selection file
        // fs::copy_file(p, target_cache_dir / "sel" / p.filename());
        fs::path to = target_sel_dir / p.filename();
        if (fs::exists(to)) fs::remove(to);
        fs::copy_file(p, to);
      }
    }
  }
}

/**
 * 1. create cpp folder
 * 2. preprocess src and put into cpp folder
 * 3. post-process cpp files
 */
void create_cpp(fs::path target_cache_dir) {
  fs::path cpp = target_cache_dir / "cpp";
  fs::path src = target_cache_dir / "src";
  if (fs::exists(cpp)) fs::remove_all(cpp);
  fs::create_directories(cpp);
  fs::recursive_directory_iterator it(src), eod;
  vector<fs::path> dirs;
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (fs::is_directory(p)) {
      dirs.push_back(p);
    }
  }

  std::string include_cmd;
  for (auto &p : dirs) {
    include_cmd += "-I" + p.string() + " ";
  }

  std::string cpp_cmd = "clang -E " + include_cmd;
    
  it = fs::recursive_directory_iterator(target_cache_dir / "src");
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (fs::is_regular_file(p)) {
      fs::path to = target_cache_dir / "cpp" / fs::relative(p, target_cache_dir / "src");
      fs::create_directories(to.parent_path());
      // redirect the clang preprocessor stderr to /dev/null
      std::string cmd = cpp_cmd + " " + p.string()
        + " >> " + to.string() + " 2>/dev/null";
      utils::new_exec(cmd.c_str());
    }
  }

  // remove extra things added by preprocessor using include files
  // std::cout << "Removing extra for cpp-ed file .." << "\n";
  it = fs::recursive_directory_iterator(target_cache_dir / "cpp");
  BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
    if (fs::is_regular_file(p)) {
      std::ifstream is;
      is.open(p.string());
      std::string code;
      if (is.is_open()) {
        std::string line;
        std::string output;
        bool b = false;
        while(getline(is, line)) {
          if (!line.empty() && line[0] == '#') {
            // this might be a line marker
            vector<string> v = utils::split(line);
            if (v.size() < 3) continue;
            string filename = v[2];
            if (filename.empty() || filename[0] != '"' || filename.back() != '"') continue;
            filename = filename.substr(1);
            filename.pop_back();
            fs::path file(filename);
            fs::path newp = fs::relative(p, target_cache_dir / "cpp");
            fs::path newfile = fs::relative(file, target_cache_dir / "src");
            if (newfile.string() == newp.string()) {
              b = true;
              // I don't need the line marker line, because clang will complain when process it
              // output += line + "\n";
            } else {
              b = false;
            }
          } else {
            if (b) {
              output += line + "\n";
            }
          }
        }
        is.close();
        // output to the same file
        utils::write_file(p.string(), output);
      }
    }
  }
}
