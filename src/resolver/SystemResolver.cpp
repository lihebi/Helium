#include "resolver/SystemResolver.hpp"
#include <iostream>
#include "type/TypeFactory.hpp"
#include "util/StringUtil.hpp"

SystemResolver* SystemResolver::m_instance = 0;

// resolve to primitive type
// uint8_t => unsigned char"
std::string
SystemResolver::ResolveType(const std::string& name) {
  // std::cout << "[SystemResolver::ResolveType] " << name << std::endl;
  std::vector<CtagsEntry> entries = Parse(name, "t");
  for (auto it=entries.begin();it!=entries.end();it++) {
    std::string pattern = it->GetPattern();
    pattern = pattern.substr(pattern.find("typedef"));
    pattern = pattern.substr(0, pattern.rfind(';'));
    // FIXME typedef xxx xxx yyy ;
    std::vector<std::string> vs = StringUtil::Split(pattern);
    // to_type is the middle part of split
    std::string to_type;
    for (int i=1;i<vs.size()-1;i++) {
      to_type += vs[i]+' ';
    }
    to_type.pop_back();
    // if it is primitive type, return
    if (TypeFactory(to_type).IsPrimitiveType()) {
      return to_type;
    }
    // depth first resolve
    std::string new_type = ResolveType(to_type);
    if (!new_type.empty()) return new_type;
  }
  return "";
}

bool
SystemResolver::Has(const std::string& name) {
  if (Parse(name).empty()) return false;
  else return true;
}

void
SystemResolver::Load(const std::string& tagfile) {
  std::cout << "[SystemResolver::Load]" << std::endl;
  tagFileInfo *info = (tagFileInfo*)malloc(sizeof(tagFileInfo));
  m_tagfile = tagsOpen (tagfile.c_str(), info);
  m_entry = (tagEntry*)malloc(sizeof(tagEntry));
}

std::vector<CtagsEntry>
SystemResolver::Parse(const std::string& name) {
  std::vector<CtagsEntry> vc;
  tagResult result = tagsFind(m_tagfile, m_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (m_entry->kind) {
      vc.push_back(CtagsEntry(name, m_entry->file, m_entry->address.pattern, *(m_entry->kind)));
    }
    // find next
    result = tagsFindNext(m_tagfile, m_entry);
  }
  return vc;
}


std::vector<CtagsEntry>
SystemResolver::Parse(const std::string& name, const std::string& type) {
  std::vector<CtagsEntry> vc;
  tagResult result = tagsFind(m_tagfile, m_entry, name.c_str(), TAG_FULLMATCH);
  while (result == TagSuccess) {
    if (m_entry->kind && type.find(*(m_entry->kind)) != -1) {
      vc.push_back(CtagsEntry(name, m_entry->file, m_entry->address.pattern, *(m_entry->kind)));
    }
    // find next
    result = tagsFindNext(m_tagfile, m_entry);
  }
  return vc;
}
