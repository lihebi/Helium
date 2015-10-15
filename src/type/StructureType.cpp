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

StructureType::StructureType(const std::string& name) {
  // need to resolve instead of looking up registry,
  // because the resolving snippet phase
  // is far behind current phase of resolve IO variables.
  Logger::Instance()->LogTrace("[StructureType::StructureType]"+name+"\n");
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(name);
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 's') {
      m_snippet = *it;
      m_name = m_snippet->GetName();
      break;
    }
  }
  assert(!m_name.empty());
  // parseFields();
}
StructureType::~StructureType() {
}

std::string
StructureType::GetInputCode(const std::string& var) const {
  std::string code;
  if (GetDimension()>0) {
    return Type::GetArrayCode(m_name, var, GetDimension());
  }
  if (GetPointerLevel()>0) {
    return Type::GetAllocateCode(m_name, var, GetPointerLevel());
  } else {
    code += m_name + " " + var + ";\n";
  }
  // fields init
  for (auto it=m_fields.begin();it!=m_fields.end();it++) {
    std::string field_name = it->second;
    field_name = std::string(GetPointerLevel()-1, '&') + var + "." + field_name;
    code += it->first->GetInputCode(field_name) + "\n";
  }
  return code;
}

std::string
StructureType::GetOutputCode(const std::string& var) const {
  return "";
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
  std::cout << "[StructureType::parseFields]" << std::endl;
  pugi::xml_document doc;
  std::cout << "/* message */" << std::endl;
  SrcmlUtil::String2XML(m_snippet->GetCode(), doc);
  std::cout << "/* message */" << std::endl;
  pugi::xml_node struct_node = doc.first_element_by_path("//struct");
  // pugi::xml_node name_node = struct_node.child("name");
  pugi::xml_node block_node = struct_node.child("block");
  for (pugi::xml_node decl_stmt_node : block_node.children("decl_stmt")) {
    std::cout << "/* message */" << std::endl;
    pugi::xml_node decl_node = decl_stmt_node.child("decl");
    std::string type_str = DomUtil::GetTextContent(decl_node.child("type"));
    std::string name_str = DomUtil::GetTextContent(decl_node.child("name"));
    if (name_str.find('[') != std::string::npos) {
      type_str += name_str.substr(name_str.find('['));
    }
    name_str = name_str.substr(0, name_str.find('['));
    std::cout << "/* message */" << std::endl;
    // FIXME this line will have recursive problem
    std::shared_ptr<Type> type = TypeFactory(type_str).CreateType();
    m_fields.push_back(std::make_pair(type, name_str));
  }
}
