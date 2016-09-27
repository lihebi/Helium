#include "resolver.h"
#include <boost/regex.hpp>
#include "utils/utils.h"
#include "parser/xml_doc_reader.h"
#include <gtest/gtest.h>
#include "parser/xmlnode_helper.h"
/**
 * Extract id which is not c keyword
 * This is the master copy of this resolving
 * The other one calls it.
 *
 * @param code [in] input code
 * @return a set of IDs
 */
std::set<std::string>
extract_id_to_resolve(std::string code) {
  // TODO move to trace --verbose
  // print_trace("extract_id_to_resolve");

  // Before doing the pattern matching, I want to first remove comments
  XMLDoc *doc = XMLDocReader::CreateDocFromString(code);
  assert(doc);
  code = get_text_except(doc->document_element(), NK_Comment);
  delete doc;
  
  static boost::regex id_reg("\\b[_a-zA-Z][_a-zA-Z0-9]*\\b");
  boost::smatch match;
  boost::sregex_iterator begin(code.begin(), code.end(), id_reg);
  boost::sregex_iterator end = boost::sregex_iterator();
  std::set<std::string> ss;
  for (boost::sregex_iterator it=begin;it!=end;it++) {
    std::string tmp = (*it).str();
    if (c_common_keywords.find(tmp) == c_common_keywords.end()) {
      ss.insert(tmp);
    }
  }
  return ss;
}

TEST(ResolverTestCase, ExtractIdTest) {
  std::string code = "[MAXPATHLEN]";
  std::set<std::string> ids = extract_id_to_resolve(code);
  ASSERT_EQ(ids.size(), 1);
  EXPECT_EQ(*ids.begin(), "MAXPATHLEN");
}

std::set<std::string>
extract_id_to_resolve(XMLNodeList nodes) {
  std::set<std::string> result;
  for (XMLNode node : nodes) {
    std::set<std::string> tmp = extract_id_to_resolve(get_text(node));
    result.insert(tmp.begin(), tmp.end());
  }
  return result;
}

std::set<std::string>
get_to_resolve(
               XMLNodeList nodes,
               std::set<std::string> known_to_resolve,
               std::set<std::string> known_not_resolve
               ) {
  std::set<std::string> result;
  std::set<std::string> var_ids = get_var_ids(nodes);
  result.insert(var_ids.begin(), var_ids.end());
  // var_ids
  // general types in the nodes
  std::set<std::string> type_ids = get_type_ids(nodes);
  result.insert(type_ids.begin(), type_ids.end());
  // call to functions
  std::set<std::string> call_ids = get_call_ids(nodes);
  result.insert(call_ids.begin(), call_ids.end());
  // constructing
  result.insert(known_to_resolve.begin(), known_to_resolve.end());
  for (const std::string &s : known_not_resolve) {
    result.erase(s);
  }
  for (const std::string &s : c_common_keywords) {
    result.erase(s);
  }
  return result;
}

/**
 * Semantic get to resolve set.
 */
std::set<std::string>
get_to_resolve(
               std::string code,
               std::set<std::string> known_to_resolve,
               std::set<std::string> known_not_resolve
               ) {
  XMLDoc doc;
  utils::string2xml(code, doc);
  XMLNodeList nodes;
  nodes.push_back(doc.document_element());
  return get_to_resolve(nodes, known_to_resolve, known_not_resolve);
}

// SymbolOldTable::SymbolOldTable() {
//   // ensure there's at least one table. Calling back() on empty vector is undefined.
//   PushLevel();
// }
// SymbolOldTable::~SymbolOldTable() {}
// int SymbolOldTable::CurrentLevel() {
//   return m_tables.size();
// }
// void SymbolOldTable::PushLevel() {
//   m_tables.push_back(std::map<std::string, Variable>());
// }
// void SymbolOldTable::PopLevel() {
//   m_tables.pop_back();
// }
// void SymbolOldTable::AddSymbol(Variable v) {
//   m_tables.back()[v.Name()] = v;
// }
// void SymbolOldTable::AddSymbol(VariableList vars) {
//   for (Variable v : vars) {
//     m_tables.back()[v.Name()] = v;
//   }
// }
// Variable SymbolOldTable::LookUp(const std::string &name) {
//   // FIXME back() will copy?
//   // FIXME performance
//   for (int i=m_tables.size()-1;i>=0;i--) {
//     if (m_tables[i].find(name) != m_tables[i].end()) {
//       return m_tables[i][name];
//     }
//   }
//   return Variable();
// }
