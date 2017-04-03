#ifndef CACHE_H
#define CACHE_H

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <string>

namespace fs = boost::filesystem;


void create_tagfile(const std::string& folder, const std::string& file);
void create_src(fs::path target, fs::path target_cache_dir, fs::path target_sel_dir);
void create_cpp(fs::path target_cache_dir);
void create_tagfile(fs::path target_cache_dir);
void create_clang_snippet(fs::path target_cache_dir);
void create_snippet_db(fs::path target_cache_dir);

#endif /* CACHE_H */
