#include "type.h"
#include "type_common.h"
#include "type_helper.h"

#include "parser/xml_doc_reader.h"


using namespace type;
using namespace utils;

/**
 * For structure type, we will analyze two levels.
 * struct A {struct A *next;}
 */
LocalStructureType::LocalStructureType(int snippet_id, std::string raw, std::vector<std::string> dims, int token)
  : Type(raw, dims), m_snippet_id(snippet_id), m_token(token) {
  // std::set<int> snippet_ids = SnippetDB::Instance()->LookUp(m_id, {SK_Enum, SK_Structure});
  // assert(snippet_ids.size() > 0);
  // assert(snippet_ids.size() == 1);
  // m_snippet_id = *snippet_ids.begin();
  // TODO use a cache
  m_code = SnippetDB::Instance()->GetCode(m_snippet_id);
  assert(!m_code.empty());
  // build fields
  buildArch();
  // DEBUG
  // std::cout << "size of fields: " << m_fields.size()  << "\n";
  // for (auto m : m_fields) {
  //   Type *t = m.first;
  //   std::string name = m.second;
  //   std::cout << "\t" << t->ToString() << " : " << name  << "\n";
  // }
  // std::cout << "...."  << "\n";
  // getchar();
}

void LocalStructureType::buildArch() {
  assert(m_token >= 0);
  if (m_token == 0) return;
  XMLDoc *doc = XMLDocReader::CreateDocFromString(m_code);
  // <struct> <block> <decl_stmt>
  XMLNode node = doc->document_element();
  node.select_nodes("//struct/block/decl_stmt");
  XMLNode struct_node = node.select_node("//struct").node();
  for (XMLNode decl_stmt : struct_node.child("block").children("decl_stmt")) {
    // get type and variable name
    XMLNodeList decls = decl_stmt_get_decls(decl_stmt);
    for (XMLNode decl : decls) {
      // IMPORTANT here decrease token by 1
      Type *t = TypeFactory::CreateType(decl, m_token-1);
      std::string name = decl_get_name(decl);
      if (t && !name.empty()) {
        m_fields.push_back({t, name});
      }
    }
  }
  delete doc;
}

std::string LocalStructureType::GetDeclCode(std::string var) {
  std::string ret;
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

/**
 * I need a more powerful local type input generation.
 * The features needed:
 * - analyze the fields and init important ones
 * - might have a shape, e.g. node->next
 */

std::string LocalStructureType::GetInputCode(std::string var) {
  std::string ret;
  if (m_pointer == 0) {
    ret += "\n// HELIUM_TODO LocalStructureType::GetInputCode, m_pointer = 0: " + m_raw + " : " + var + "\n";
  } else if (m_pointer == 1) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    ret += "\n// LocalStructureType::GetInputCode : " + m_raw + " : " + var + "\n";
    // this is 0 or 1, whether NULL or not null
    // FIXME so the generation of input should conform to this
    // FIXME is it safe to use helium_size? Nested one might change the value!
    if (m_token == 0) {
      // Init it into NULL
      ret += var + " = NULL;\n";
    } else {
      std::string tmp;
      tmp += get_malloc_code(var, m_raw_without_pointer, "1");
      if (Config::Instance()->GetBool("instrument-address")) {
        tmp += get_addr_input(var);
      }
      tmp += "// init the fields\n";
      for (auto m : m_fields) {
        Type *t = m.first;
        std::string name = m.second;
        tmp += t->GetInputCode(var + "->" + name);
      }
      ret += get_scanf_code("%d", "&helium_size");
      ret += get_helium_size_branch(
                                    // false branch, helium_size == 0
                                    var + " = NULL;\n",
                                    // get_malloc_code(var, m_raw_without_pointer, "1")
                                    tmp
                                    );
    }
    
  } else {
    ret += "\n// HELIUM_TODO LocalStructureType::GetInputCode double pointer\n";
  }
  return ret;
}

std::string LocalStructureType::GetOutputCode(std::string var) {
  std::string ret;
  if (m_pointer == 0) {
    ret += "\n// HELIUM_TODO LocalStructureType::GetOutputCode\n";
  } else if (m_pointer == 1) {
    // if (Config::Instance()->GetBool("instrument-null")) {
    //   ret += get_check_null(var,
    //                         get_null_output(var, true),
    //                         get_null_output(var, false)
    //                         );
    // }

    // 1. check null
    // 2. if not null, output fields
    std::string addr_output;
    if (Config::Instance()->GetBool("instrument-address")) {
      addr_output = get_addr_output(var);
    }
    std::string field_output;
    for (auto m : m_fields) {
      Type *t = m.first;
      std::string name = m.second;
      field_output += t->GetOutputCode(var + "->" + name);
    }
    ret += get_check_null(var,
                          get_null_output(var, true),
                          get_null_output(var, false) + addr_output + field_output
                          );
  } else {
    ret += "\n// HELIUM_TODO LocalStructureType::GetOutputCode\n";
  }
  return ret;
}


TestInput* LocalStructureType::GetTestInputSpec(std::string var) {
  LocalStructTestInput *ret = new LocalStructTestInput(this, var);
  std::string raw;
  // if token is 0, then just no need input
  assert(m_pointer < 2);
  if (m_pointer == 0) {
    ret->SetRaw("");
    ret->SetNull(NULL_NA);
    return ret;
  }
  if (m_token == 0) {
    ret->SetRaw("");
    ret->SetNull(NULL_True);
    return ret;
  }
  ret->SetRaw("");
  ret->SetNull(NULL_NA);
  bool is_null = utils::rand_bool();
  if (is_null) {
    ret->SetNull(NULL_True);
    raw += "0";
  } else {
    ret->SetNull(NULL_False);
    raw += "1";
    for (auto m : m_fields) {
      Type *t = m.first;
      std::string name = m.second;
      TestInput *in = t->GetTestInputSpec(var + "->" + name);
      ret->AddField(in);
      raw += " " + in->GetRaw();
    }
  }
  ret->SetRaw(raw);
  return ret;
}

/**
 * For human read only
 */
std::string LocalStructTestInput::dump() {
  std::string ret;
  assert(m_type);
  ret += "LocalStruct: " + m_type->ToString()
    + " : " + m_var + "\n";
  return ret;
}

std::string LocalStructTestInput::ToString() {
  std::string ret;
  if (m_type->Pointer() == 0) {
    // TODO fields only
    return "";
  } else if (m_type->Pointer() == 1) {
    // 1. if pointer, NULL or not
    ret += "In_" + m_var + " = ";
    switch (m_null) {
    case NULL_NA: return "";
    case NULL_True: ret += "NULL\n"; break;
    case NULL_False: ret += "!NULL\n"; break;
    default: assert(false);
    }
    // 2. Fields
    // assert(m_type->Kind() == NTK_LocalStructure);
    // for (auto m : dynamic_cast<LocalStructureType*>(m_type)->Fields()) {
    //   Type *t = m.first;
    //   std::string var = m.second;
    //   TestInput *in = t->GetTestInputSpec(m_var + "->" + var);
    //   ret += in->ToString();
    // }
    for (TestInput *in : m_field_inputs) {
      assert(in);
      ret += in->ToString();
    }
  } else {
    // do not support double pointer
    return "";
  }
  return ret;
}
