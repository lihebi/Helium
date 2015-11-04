#include "resolver/Ctags.hpp"
#include "util/FileUtil.hpp"
#include "snippet/SnippetRegistry.hpp"
#include "Logger.hpp"
#include <iostream>
#include <unistd.h>

#include <cstdlib>

Ctags* Ctags::m_instance = 0;

void
Ctags::Load(const std::string& tagfile) {
  Logger::Instance()->LogTrace("[Ctags::Load]\n");
  tagFileInfo *info = (tagFileInfo*)malloc(sizeof(tagFileInfo));
  m_tagfile = tagsOpen (tagfile.c_str(), info);
  // this is a int ... upon success, it will be set to 1
  if (info->status.opened != true) {
    std::cerr << "[Ctags::Load] cannot load tagfile: " << tagfile
    << ". Did you forget to preprocess the benchmark?"<< std::endl;
    exit(1);
  }
  m_entry = (tagEntry*)malloc(sizeof(tagEntry));
}

std::vector<CtagsEntry>
Ctags::Parse(const std::string& name) {
  std::vector<CtagsEntry> vc;
  tagResult result = tagsFind(m_tagfile, m_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (m_entry->kind) {
      vc.push_back(CtagsEntry(name, m_entry->file, m_entry->address.lineNumber, *(m_entry->kind)));
    }
    // find next
    result = tagsFindNext(m_tagfile, m_entry);
  }
  return vc;
}

std::vector<CtagsEntry>
Ctags::Parse(const std::string& name, const std::string& type) {
  std::vector<CtagsEntry> vc;
  tagResult result = tagsFind(m_tagfile, m_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (m_entry->kind && type.find(*(m_entry->kind)) != std::string::npos) {
      vc.push_back(CtagsEntry(name, m_entry->file, m_entry->address.lineNumber, *(m_entry->kind)));
    }
    // find next
    result = tagsFindNext(m_tagfile, m_entry);
  }
  return vc;
}

// load and register in SnippetRegistry
// return Snippet*
std::set<Snippet*>
Ctags::Resolve(const std::string& name) {
  // std::cout << "[Ctags::Resolve] " << name << std::endl;
  // TODO look up first
  std::vector<CtagsEntry> vc = Parse(name);
  // std::cout << "[Ctags::Resolve] parsed size: " << vc.size() << std::endl;
  std::set<Snippet*> vsp;
  for (auto it=vc.begin();it!=vc.end();it++) {
    Snippet *snippet = SnippetRegistry::Instance()->Add(*it);
    if (snippet) {
      vsp.insert(snippet);
    }
  }
  return vsp;
}
std::set<Snippet*>
Ctags::Resolve(const std::string& name, const std::string& type) {
  std::vector<CtagsEntry> vc = Parse(name, type);
  std::set<Snippet*> vsp;
  for (auto it=vc.begin();it!=vc.end();it++) {
    Snippet *snippet = SnippetRegistry::Instance()->Add(*it);
    vsp.insert(snippet);
  }
  return vsp;
}
