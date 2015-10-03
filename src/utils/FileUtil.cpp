#include "util/FileUtil.hpp"
#include "util/StringUtil.hpp"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>

namespace fs = boost::filesystem;

std::vector<std::string> FileUtil::GetFiles(const std::string& folder) {
  std::vector<std::string> vs;
  GetFiles(folder, vs);
  return vs;
}
// get file of the folder, store in vs
void FileUtil::GetFiles(const std::string& folder, std::vector<std::string>& vs) {
  fs::path project_folder(folder);
  fs::recursive_directory_iterator it(folder), eod;
  BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      vs.push_back(p.string());
    }
  }
}

void FileUtil::GetFilesByExtension(
  const std::string& folder,
  std::vector<std::string>& vs,
  const std::string& extension
) {
  std::vector<std::string> ve {extension};
  GetFilesByExtension(folder, vs, ve);
}

void FileUtil::GetFilesByExtension(
  const std::string& folder,
  std::vector<std::string>& vs,
  const std::vector<std::string>& extension
) {
  fs::path project_folder(folder);
  fs::recursive_directory_iterator it(folder), eod;
  BOOST_FOREACH(fs::path const& p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      std::string with_dot = p.extension().string();
      // the extension may not exist
      if (!with_dot.empty()) {
        // if extension does not exist,
        // the substr(1) will cause exception of out of range of basic_string
        std::string without_dot = with_dot.substr(1);
        if (std::find(extension.begin(), extension.end(), with_dot) != extension.end()
        || std::find(extension.begin(), extension.end(), without_dot) != extension.end()) {
          vs.push_back(p.string());
        }
      }
    }
  }
}

// from line, match a {}
std::string
extract_forward(const std::vector<std::string>& vs, int line_number) {
  std::string s = "";
  int open_brace_count = 0;
  int close_brace_count = 0;
  for (int i=line_number;i<vs.size();i++) {
    s += vs[i]+'\n';
    open_brace_count += std::count(vs[i].begin(), vs[i].end(), '{');
    close_brace_count += std::count(vs[i].begin(), vs[i].end(), '}');
    if (open_brace_count == close_brace_count && open_brace_count > 0) break;
  }
  return s;
}

// extract backward for a match {}
std::string
extract_backward(const std::vector<std::string>& vs, int line_number) {
  std::string s;
  int open_brace_count = 0;
  int close_brace_count = 0;
  for (int i=line_number;i>0;i--) {
    s = vs[i] + '\n' + s;
    open_brace_count += std::count(vs[i].begin(), vs[i].end(), '{');
    close_brace_count += std::count(vs[i].begin(), vs[i].end(), '}');
    if (open_brace_count == close_brace_count && open_brace_count > 0) break;
  }
  return s;
}

// from line, match until doesn't ends with `\`
std::string
extract_define(const std::vector<std::string>& vs, int line_number) {
  std::string s;
  for (int i=line_number;i<vs.size();i++) {
    s += vs[i] + '\n';
    std::string tmp = vs[i];
    StringUtil::trim(tmp);
    if (tmp.back() != '\\') break;
  }
  return s;
}
// until ends with ;
std::string
extract_statement(const std::vector<std::string>& vs, int line_number) {
  std::string s;
  for (int i=line_number; i<vs.size();i++) {
    s += vs[i] + '\n';
    // FIXME this may be expensive
    std::string tmp = vs[i];
    StringUtil::trim(tmp);
    if (tmp.back() == ';') break;
  }
  return s;
}
// from middle, extract double direction until { and }
std::string
extract_double(const std::vector<std::string>& vs, int line_number) {
  int i;
  for (i=line_number;i>0;i--) {
    if (vs[i].find('{') != -1) break;
  }
  std::string s;
  for (;i<vs.size();i++) {
    s += vs[i] + '\n';
    if (vs[i].find('}') != -1) break;
  }
  return s;
}
// if start with 'typedef', return this line_number
// otherwise backward
std::string
extract_typedef(const std::vector<std::string>& vs, int line_number) {
  std::string tmp = vs[line_number];
  StringUtil::trim(tmp);
  if (StringUtil::StartsWith(tmp, "typedef")) {
    // this is the head of typedef. May contain multiple lines. Match until ;
    tmp = vs[line_number];
    StringUtil::trim(tmp);
    std::string code;
    while (tmp.back() != ';') {
      code += vs[line_number] + "\n";
      line_number++;
      tmp = vs[line_number];
      StringUtil::trim(tmp);
    }
    code += vs[line_number] + "\n";
    return code;
  } else {
    // this is at the end of typedef, the alias
    // FIXME maybe typedef xxx \n xxx alias;
    return extract_backward(vs, line_number);
  }
}


std::string
FileUtil::GetBlock(const std::string& filename, int line, char type) {
  line--; // vector index from 0, but file line from 1
  std::ifstream is;
  std::vector<std::string> lines;
  is.open(filename);
  if (is.is_open()) {
    std::string line;
    while (getline(is, line)) {
      lines.push_back(line);
    }
    is.close();
  }
  switch(type) {
    case 'f':
    case 's':
    case 'g':
    case 'u': return extract_forward(lines, line); break;
    case 'd': return extract_define(lines, line); break;
    case 'v': return extract_statement(lines, line); break;
    case 'e': return extract_double(lines, line); break;
    case 't': return extract_typedef(lines, line); break;
    default: return "";
  }
}


void
FileUtil::Write(
  const std::string& file,
  const std::string& content
) {
  fs::path file_path(file);
  fs::path dir = file_path.parent_path();
  if (!fs::exists(dir)) {
    fs::create_directories(dir);
  }
  std::ofstream os;
  os.open(file_path.string());
  if (os.is_open()) {
    os << content;
    os.close();
  }
}

std::string
FileUtil::Read(const std::string& file) {
  std::ifstream is;
  is.open(file);
  std::string code;
  if (is.is_open()) {
    std::string line;
    while(getline(is, line)) {
      code += line+"\n";
    }
    is.close();
  }
  return code;
}

void
FileUtil::RemoveFolder(const std::string& folder) {
  fs::path folder_path(folder);
  if (fs::exists(folder_path)) {
    fs::remove_all(folder_path);
  }
}

void
FileUtil::CreateFolder(const std::string& folder) {
  fs::path folder_path(folder);
  if (!fs::exists(folder_path)) {
    fs::create_directories(folder_path);
  }
}
