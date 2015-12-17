#include "type/StructureType.hpp"
#include <pugixml.hpp>
#include "util/SrcmlUtil.hpp"
#include <set>
#include "snippet/SnippetRegistry.hpp"
#include "util/DomUtil.hpp"
#include "type/TypeFactory.hpp"
#include <iostream>
#include "resolver/Ctags.hpp"
#include <cassert>
#include "Logger.hpp"
#include "variable/VariableFactory.hpp"

std::set<std::string> StructureType::m_recursion_set = std::set<std::string>();

StructureType::StructureType(const std::string& name) {
  // need to resolve instead of looking up registry,
  // because the resolving snippet phase
  // is far behind current phase of resolve IO variables.
  Logger::Instance()->LogTraceV("[StructureType::StructureType]"+name+"\n");
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(name);
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 's') {
      m_snippet = *it;
      m_name = m_snippet->GetName();
      break;
    }
  }
  assert(!m_name.empty());
  // enable parseFields will cause "fork: resource temporary unavailable", even on memcached.
  // not sure why, but it should be something about ThreadUtil
  // However threadutil is synchronized, so it should not have problem.
  // As a result, temporarily this parse field feature cannot be enabled.
  // But I'm pretty sure for large benchmarks this error should also occur.

  // if (m_recursion_set.find(m_name) != m_recursion_set.end()) {
  //   // if recursion detected, do not parse the field in current type, and init value set to null
  //   m_null = true;
  //   Logger::Instance()->LogDebug("[StructureType::StructureType] " + m_name + " is recursion, set to null.\n");
  //   return;
  // } else {
  //   // before parseFields, push self to the stack
  //   m_recursion_set.insert(m_name);
  //   parseFields();
  //   // after parseFields, pop self from stack
  //   m_recursion_set.erase(m_name);
  //   Logger::Instance()->LogDebug(
  //     "[StructureType::StructureType] current recursion_set size: "
  //     +std::to_string(m_recursion_set.size()) + '\n'
  //   );
  // }
}
StructureType::~StructureType() {
}

std::string
StructureType::GetInputCode(const std::string& var) const {
  Logger::Instance()->LogTraceV("[StructureType::GetInputCode]\n");
  std::string code;
  if (GetDimension()>0) {
    code += Type::GetArrayCode(m_name, var, GetDimension());
  }
  if (GetPointerLevel()>0) {
    code += Type::GetDeclCode(m_name, var, GetPointerLevel());
    code += Type::GetAllocateCode(m_name, var, GetPointerLevel());
  } else {
    code += m_name + " " + var + ";\n";
  }
  // fields init
  for (auto it=m_fields.begin();it!=m_fields.end();it++) {
    assert(GetPointerLevel()>=0);
    std::string prefix;
    if (GetPointerLevel()>0) {
      prefix = "("+std::string(GetPointerLevel(), '*')+var+").";
    } else {
      prefix = var+'.';
    }
    code += (*it)->GetInputCodeWithoutDecl(prefix);
  }
  return code;
}

std::string
StructureType::getPrefix(const std::string& var) const {
  std::string prefix;
  if (GetPointerLevel()>0) {
    prefix = "("+std::string(GetPointerLevel(), '*')+var+").";
  } else {
    prefix = var+'.';
  }
  return prefix;
}

std::string
StructureType::GetInputCodeWithoutDecl(const std::string& var) const {
  if (m_null) {
    return var + " = NULL;\n";
  }
  std::string code;
  if (GetPointerLevel()>0) {
    code += Type::GetAllocateCode(m_name, var, GetPointerLevel());
  }
  for (auto it=m_fields.begin();it!=m_fields.end();it++) {

    code += (*it)->GetInputCodeWithoutDecl(getPrefix(var));
  }
  return code;
}

std::string
StructureType::GetOutputCode(const std::string& var) const {
  Logger::Instance()->LogTraceV("[StructureType::GetOutputCode]\n");
  std::string code;
  if (GetDimension() > 0) {
    code += "// [StructureType::GetOutputCode] array code omitted.\n";
    return code;
  }
  if (GetPointerLevel()>0) {
    code += "printf(\"%d\", "+var+"==NULL);\n";
  }
  for (auto it=m_fields.begin();it!=m_fields.end();it++) {
    code += (*it)->GetOutputCode(getPrefix(var));
  }
  return code;
}
std::string
StructureType::GetInputSpecification() {
  return "";
}
std::string
StructureType::GetOutputSpecification() {
  return "";
}

void
StructureType::parseFields() {
  Logger::Instance()->LogTraceV("[StructureType::parseFields]\n");
  pugi::xml_document doc;
  SrcmlUtil::String2XML(m_snippet->GetCode(), doc);
  pugi::xml_node struct_node = doc.select_node("//struct").node();
  // pugi::xml_node name_node = struct_node.child("name");
  pugi::xml_node block_node = struct_node.child("block");
  for (pugi::xml_node decl_stmt_node : block_node.children("decl_stmt")) {
    std::shared_ptr<Variable> v = VariableFactory::FromDeclStmt(decl_stmt_node);
    if (v) {
      m_fields.push_back(v);
    }
  }
}
