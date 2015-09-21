#include "resolver/Ctags.hpp"
#include "util/FileUtil.hpp"
#include "snippet/SnippetRegistry.hpp"
#include <iostream>

#include <cstdlib>

Ctags* Ctags::m_instance = 0;

void
Ctags::Load(const std::string& tagfile) {
  std::cout << "[Ctags::Load]" << std::endl;
  tagFileInfo *info = (tagFileInfo*)malloc(sizeof(tagFileInfo));
  m_tagfile = tagsOpen (tagfile.c_str(), info);
  m_entry = (tagEntry*)malloc(sizeof(tagEntry));
}


CtagsEntry
Ctags::ParseSimple(const std::string& name) {
  tagResult result = tagsFind(m_tagfile, m_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (m_entry->kind) {
      return CtagsEntry(m_entry->file, m_entry->address.lineNumber, *(m_entry->kind));
    }
    result = tagsFindNext(m_tagfile, m_entry);
  }
  return CtagsEntry(false);
}

std::vector<CtagsEntry>
Ctags::Parse(const std::string& name) {
  std::vector<CtagsEntry> vc;
  tagResult result = tagsFind(m_tagfile, m_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (m_entry->kind) {
      vc.push_back(CtagsEntry(m_entry->file, m_entry->address.lineNumber, *(m_entry->kind)));
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
    if (m_entry->kind && type.find(*(m_entry->kind)) != -1) {
      vc.push_back(CtagsEntry(m_entry->file, m_entry->address.lineNumber, *(m_entry->kind)));
    }
    // find next
    result = tagsFindNext(m_tagfile, m_entry);
  }
  return vc;
}

Snippet*
Ctags::ResolveSimple(const std::string& name) {
  CtagsEntry ce = ParseSimple(name);
  if (ce) {
    std::string code = FileUtil::GetBlock(ce.GetFileName(), ce.GetLineNumber(), ce.GetType());
    Snippet *snippet = SnippetRegistry::Instance()->Add(code, ce.GetType(), name);
    return snippet;
  }
  return NULL;
}

// load and register in SnippetRegistry
// return Snippet*
std::set<Snippet*>
Ctags::Resolve(const std::string& name) {
  // std::cout << "[Ctags::Resolve] " << name << std::endl;
  std::vector<CtagsEntry> vc = Parse(name);
  // std::cout << "[Ctags::Resolve] parsed size: " << vc.size() << std::endl;
  std::set<Snippet*> vsp;
  for (auto it=vc.begin();it!=vc.end();it++) {
    std::string code = FileUtil::GetBlock(it->GetFileName(), it->GetLineNumber(), it->GetType());
    Snippet *snippet = SnippetRegistry::Instance()->Add(code, it->GetType(), name);
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
    std::string code = FileUtil::GetBlock(it->GetFileName(), it->GetLineNumber(), it->GetType());
    Snippet *snippet = SnippetRegistry::Instance()->Add(code, it->GetType(), name);
    vsp.insert(snippet);
  }
  return vsp;
}
