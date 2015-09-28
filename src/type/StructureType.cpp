#include "type/StructureType.hpp"
#include <pugixml/pugixml.hpp>
#include "util/SrcmlUtil.hpp"
#include <set>
#include "snippet/SnippetRegistry.hpp"
#include "util/DomUtil.hpp"
#include "type/TypeFactory.hpp"
#include <iostream>
#include "resolver/Ctags.hpp"
#include <cassert>

StructureType::StructureType(const std::string& name) : m_name(name) {
  std::cout << "[StructureType::StructureType] " << m_name << std::endl;
  Snippet *s = SnippetRegistry::Instance()->LookUp(name, 't');
  // FIXME may not be the first match
  m_snippet = s;
  // TODO
  // parseFields();
}
StructureType::~StructureType() {
}

std::string
StructureType::GetInputCode(const std::string& var) const {
  std::string code;
  if (m_dimension>0) {
    return Type::GetArrayCode(m_name, var, m_dimension);
  }
  if (m_pointer_level>0) {
    // std::string var_tmp = var + "_tmp";
    // code += m_name + "* " + var_tmp + " = (" + m_name + "*)malloc(sizeof(" + m_name + "));\n";
    // code += m_name + std::string(m_pointer_level, '*')+ " " + var
    // + " = " + std::string(m_pointer_level-1, '&') + var_tmp + ";\n";
    return Type::GetAllocateCode(m_name, var, m_pointer_level);
  } else {
    code += m_name + " " + var + ";\n";
  }
  // fields init
  for (auto it=m_fields.begin();it!=m_fields.end();it++) {
    std::string field_name = it->second;
    field_name = std::string(m_pointer_level-1, '&') + var + "." + field_name;
    code += it->first->GetInputCode(field_name) + "\n";
  }
  return code;
}

std::string
StructureType::GetOutputCode(const std::string& var) const {
  return "";
}
void StructureType::GetInputSpecification() {
}
void StructureType::GetOutputSpecification() {
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
    if (name_str.find('[') != -1) {
      type_str += name_str.substr(name_str.find('['));
    }
    name_str = name_str.substr(0, name_str.find('['));
    std::cout << "/* message */" << std::endl;
    // FIXME this line will have recursive problem
    std::shared_ptr<Type> type = TypeFactory(type_str).CreateType();
    m_fields.push_back(std::make_pair(type, name_str));
  }
}
