#include "type.h"

#include "type_common.h"
#include "type_helper.h"

#include <gtest/gtest.h>

using namespace utils;
using namespace ast;
using namespace type;

/********************************
 * Type factory
 *******************************/

Type::Type(std::string raw, std::vector<std::string> dims) : m_raw(raw), m_dims(dims) {
  m_id = get_id(m_raw);
  // decompose, get the pointer and dimension
  m_pointer = count_and_remove(raw, '*');
  // if the dims contains empty string, that means char *argv[],
  // then, remove that empty string, and add pointer level
  // so it becomes char** argv
  /**
   * HEY, in this example, the remove idiom should be, evaluate m_dims.end() EVERY TIME!
   * The erase will change the end, so you cannot just evaluate it one time in init expr of the for.
   */
  for (auto it=m_dims.begin();it!=m_dims.end();) {
    if (it->empty()) {
      it = m_dims.erase(it);
      m_pointer++;
      m_raw += '*';
    } else {
      ++it;
    }
  }
  m_dimension = m_dims.size();
  // without pointer
  utils::trim(m_raw);
  m_raw_without_pointer = m_raw;
  while (m_raw_without_pointer.back() == '*') {
    m_raw_without_pointer.pop_back();
    utils::trim(m_raw_without_pointer);
  }
}

/**
 * The return value should be free-d by user.
 */
TestInput* Type::GetTestInputSpec(std::string var) {
  TestInput *ret = new TestInput(this, var);
  ret->SetRaw("");
  // std::cout << ret->GetRaw()  << "\n";
  // std::cout << GetTestInput()  << "\n";
  // getchar();
  return ret;
}




// static struct storage_specifier get_storage_specifier(std::string raw_type) {
//   struct storage_specifier ret;
//   fill_storage_specifier(raw_type, ret);
//   return ret;
// }

static struct type_specifier get_type_specifier(std::string raw) {
  struct type_specifier ret;
  fill_type_specifier(raw, ret);
  return ret;
}
// static struct type_qualifier get_type_qualifier(std::string raw) {
//   struct type_qualifier ret;
//   fill_type_qualifier(raw, ret);
//   return ret;
// }
// static struct struct_specifier  get_struct_specifier(std::string raw) {
//   struct struct_specifier ret;
//   fill_struct_specifier(raw, ret);
//   return ret;
// }

Type* TypeFactory::CreateType(std::string raw, std::vector<std::string> dims, int token) {
  // print_trace("TypeFactory::CreateType: " +  raw);
  if (raw.empty()) return NULL;
  std::string id = get_id(raw);
  if (id.empty()) {
    struct type_specifier ts = get_type_specifier(raw);
    if (ts.is_void) {
      return new Void(raw, dims);
    } else if (ts.is_char) {
      return new Char(raw, dims);
    } else if (ts.is_float || ts.is_double) {
      return new Float(raw, dims);
    } else if (ts.is_bool) {
      return new Bool(raw, dims);
    } else if (ts.is_short || ts.is_long || ts.is_int || ts.is_signed || ts.is_unsigned) {
      return new Int(raw, dims);
    } else {
      std::cerr << "WW: type not created" << '\n';
      // std::cout << raw  << "\n";
      // std::cout << "Debug pause: enter to continue"  << "\n";
      // getchar();
      return NULL;
      // FIXME might be enum!
    }
  } else {
    // FIXME should do local resolving first, but since I have redefine problem, use this temporarily
    /********************************
     * System Resolving
     *******************************/
    // at this point, if the previous solution can resolve, it is already resolved
    if (SystemResolver::Instance()->Has(id)) {
      // TODO replace this also with a database?
      std::string new_type = SystemResolver::Instance()->ResolveType(id);
      if (!new_type.empty()) {
        std::string tmp = raw;
        tmp.replace(tmp.find(id), id.length(), new_type);
        return TypeFactory::CreateType(tmp, dims);
      } else {
        return new SystemType(raw, dims);
      }
    }

    /********************************
     * Local Resolving
     *******************************/

    /**
     * Querying the snippet db database
     */
    std::set<int> snippet_ids = SnippetDB::Instance()->LookUp(id);
    for (int snippet_id : snippet_ids) {
      SnippetMeta meta = SnippetDB::Instance()->GetMeta(snippet_id);
      if (meta.HasKind(SK_Structure)) {
        return new LocalStructureType(snippet_id, raw, dims, token);
      }
      if (meta.HasKind(SK_Enum)) {
        return new LocalEnumType(snippet_id, raw, dims);
      }
      if (meta.HasKind(SK_Typedef)) {
        // FIXME right now just get the typedefine-d string
        // REMOVE the star!
        // and retain the dims
        std::string code = SnippetDB::Instance()->GetCode(snippet_id);
        // FIXME removing the '*'
        utils::trim(raw);
        while (raw.back() == '*') {
          raw.pop_back();
          utils::trim(raw);
        }
        // std::cout << raw  << "\n";
        // assert(code.find("typedef") != std::string::npos);
        // assert(code.find(raw) != std::string::npos);
        if (code.find("typedef") == std::string::npos || code.find(raw) == std::string::npos) {
          return NULL;
        }
        code = code.substr(code.find("typedef") + strlen("typedef"));

        
        code = code.substr(0, code.find(raw));
        utils::trim(code);
        // while (code.back() == '*') {
        //   code.pop_back();
        //   utils::trim(code);
        // }
        return TypeFactory::CreateType(code, dims, token);
      }
    }
  }

  // should not reach here
  // FIXME TODO handle the case where the type is not created correctly (NULL)
  // It should be in model.cc, in Decl class
  std::cout << "WW: should not reach here: TypeFactory::CreateType"  << "\n";
  // std::cout << raw  << "\n";
  // DEBUG
  // assert(false);
  return NULL;
}
Type* TypeFactory::CreateType(ast::XMLNode decl_node, int token) {
  std::string type = decl_get_type(decl_node);
  std::vector<std::string> dims = decl_get_dimension(decl_node);
  return TypeFactory::CreateType(type, dims, token);
}


/********************************
 * TestInputSpec
 *******************************/

std::string TestInput::dump() {
  std::string ret;
  ret += "Default TestInput: ";
  // assert(m_type); // ArgVTestInput's m_type is NULL
  if (m_type) {
    ret += m_type->ToString() + " " + m_var + "\n";
  }
  return ret;
}

std::string TestInput::ToString() {
  std::string ret;
  ret += + "Ix_" + m_var + " = Default\n";
  return ret;
}


// /**
//  * Get decl code for a pointer type
//  */
// std::string
// get_decl_code(const std::string& type_name, const std::string& var_name, int pointer_level) {
//   return type_name + std::string(pointer_level, '*')+ " " + var_name+";\n";
// }

// static std::string
// qualify_var_name(const std::string& varname) {
//   std::string tmp = varname;
//   tmp.erase(std::remove(tmp.begin(), tmp.end(), '.'), tmp.end());
//   tmp.erase(std::remove(tmp.begin(), tmp.end(), '>'), tmp.end());
//   tmp.erase(std::remove(tmp.begin(), tmp.end(), '-'), tmp.end());
//   return tmp;
// }
// /**
//  * Only get the allocate code(malloc, assign), but no decl code.
//  */
// std::string
// get_allocate_code(const std::string& type_name, const std::string& var_name, int pointer_level) {
//   std::string code;
//   std::string var_tmp = qualify_var_name(var_name) + "_tmp";
//   code += type_name + "* " + var_tmp + " = (" + type_name + "*)malloc(sizeof(" + type_name + "));\n";
//   code += var_name + " = " + std::string(pointer_level-1, '&') + var_tmp + ";\n";
//   return code;
// }

