#include "new_type.h"
#include "utils.h"
#include "resolver.h"
#include <iostream>

using namespace utils;
using namespace ast;

static bool
search_and_remove(std::string &s, boost::regex reg) {
  if (boost::regex_search(s, reg)) {
    s = boost::regex_replace<boost::regex_traits<char>, char>(s, reg, "");
    return true;
  }
  return false;
}

static int
count_and_remove(std::string &s, char c) {
  int count = std::count(s.begin(), s.end(), c);
  if (count) {
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
  }
  return count;
}

static void
fill_storage_specifier(std::string& name, struct storage_specifier& specifier) {
  specifier.is_auto     = search_and_remove(name, boost::regex("\\bauto\\b"))     ? 1 : 0;
  specifier.is_register = search_and_remove(name, boost::regex("\\bregister\\b")) ? 1 : 0;
  specifier.is_static   = search_and_remove(name, boost::regex("\\bstatic\\b"))   ? 1 : 0;
  specifier.is_extern   = search_and_remove(name, boost::regex("\\bextern\\b"))   ? 1 : 0;
  // specifier.is_typedef     = search_and_remove(name, typedef_regex)     ? 1 : 0;
}

static void
fill_type_specifier(std::string& name, struct type_specifier& specifier) {
  specifier.is_void     = search_and_remove(name, boost::regex("\\bvoid\\b"))     ? 1 : 0;
  specifier.is_char     = search_and_remove(name, boost::regex("\\bchar\\b"))     ? 1 : 0;
  specifier.is_short    = search_and_remove(name, boost::regex("\\bshort\\b"))    ? 1 : 0;
  specifier.is_int      = search_and_remove(name, boost::regex("\\bint\\b"))      ? 1 : 0;
  specifier.is_long     = search_and_remove(name, boost::regex("\\blong\\b"))     ? 1 : 0;
  specifier.is_float    = search_and_remove(name, boost::regex("\\bfloat\\b"))    ? 1 : 0;
  specifier.is_double   = search_and_remove(name, boost::regex("\\bdouble\\b"))   ? 1 : 0;
  specifier.is_signed   = search_and_remove(name, boost::regex("\\bsigned\\b"))   ? 1 : 0;
  specifier.is_unsigned = search_and_remove(name, boost::regex("\\bunsigned\\b")) ? 1 : 0;
  specifier.is_bool     = search_and_remove(name, boost::regex("\\bbool\\b"))     ? 1 : 0;
}
static void
fill_type_qualifier(std::string& name, struct type_qualifier& qualifier) {
  qualifier.is_const    = search_and_remove(name, boost::regex("\\bconst\\b"))    ? 1 : 0;
  qualifier.is_volatile = search_and_remove(name, boost::regex("\\bvolatile\\b")) ? 1 : 0;
}

static void
fill_struct_specifier(std::string& name, struct struct_specifier& specifier) {
  specifier.is_struct = search_and_remove(name, boost::regex("\\bstruct\\b")) ? 1 : 0;
  specifier.is_enum   = search_and_remove(name, boost::regex("\\benum\\b"))   ? 1 : 0;
  specifier.is_union  = search_and_remove(name, boost::regex("\\bunion\\b"))  ? 1 : 0;
}


/********************************
 * Type factory
 *******************************/

NewType::NewType(std::string raw, std::vector<std::string> dims) : m_raw(raw), m_dims(dims) {
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
}


/**
 * Get the raw ID of the type
 */
static std::string get_id(std::string raw_type) {
  count_and_remove(raw_type, '*');
  struct storage_specifier ss;
  fill_storage_specifier(raw_type, ss);
  struct type_specifier ts;
  fill_type_specifier(raw_type, ts);
  struct type_qualifier tq;
  fill_type_qualifier(raw_type, tq);
  struct struct_specifier ss2;
  fill_struct_specifier(raw_type, ss2);
  trim(raw_type);
  return raw_type;
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

NewType* NewTypeFactory::CreateType(std::string raw, std::vector<std::string> dims) {
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
    } else {
      return new Int(raw, dims);
    }
  } else if (SystemResolver::Instance()->Has(id)) {
    // TODO replace this also with a database?
    std::string new_type = SystemResolver::Instance()->ResolveType(id);
    if (!new_type.empty()) {
      std::string tmp = raw;
      tmp.replace(tmp.find(id), id.length(), new_type);
      return NewTypeFactory::CreateType(tmp, dims);
    } else {
      return new SystemNewType(raw, dims);
    }
  } else {
    std::set<Snippet*> snippets = SnippetRegistry::Instance()->Resolve(id);
    // TODO check type
    if (snippets.empty()) {
      // FIXME why empty?
      // utils::print("warning: unknown NewType Kind", utils::CK_Red);
      // utils::print(raw, utils::CK_Red);
      assert(false);
    } else {
      // TODO store snippet pointer
      return new LocalNewType(raw, dims);
    }
  }
  // should not reach here
  return NULL;
}
NewType* NewTypeFactory::CreateType(ast::XMLNode decl_node) {
  std::string type = decl_get_type(decl_node);
  std::vector<std::string> dims = decl_get_dimension(decl_node);
  return NewTypeFactory::CreateType(type, dims);
}

/*******************************
 ** Inputing
 *******************************/

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

/********************************
 * Sub classes of Type
 *******************************/

std::string SystemNewType::GetDeclCode(std::string var) {
  std::string ret;
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string SystemNewType::GetInputCode(std::string var) {
  std::string ret;
  ret += "\n// HELIUM_TODO SystemNewType::GetInputCode\n";
  return ret;
}

std::string SystemNewType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "\n// HELIUM_TODO SystemNewType::GetOutputCode\n";
  return ret;
}

std::string SystemNewType::GetTestInput() {
  std::string ret;
  return ret;
}

std::string LocalNewType::GetDeclCode(std::string var) {
  std::string ret;
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string LocalNewType::GetInputCode(std::string var) {
  std::string ret;
  ret += "\n// HELIUM_TODO LocalNewType::GetInputCode\n";
  return ret;
}

std::string LocalNewType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "\n// HELIUM_TODO LocalNewType::GetOutputCode\n";
  return ret;
}

std::string LocalNewType::GetTestInput() {
  std::string ret;
  return ret;
}


/********************************
 * Models
 *******************************/

/**
 *

 Not let's talk about the input generation of different types.

 1. value
 int a;
 scanf("%d", &a);

 2. pointer
 int *a;
 a = (int*)malloc(sizeof(int)*helium_size);

 3. array
 int a[MAX];

 4. array of pointers
 char *argv[];
 scanf("%s", &argv[0]);
 scanf("%s", &argv[1]);
 */


std::string Int::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Int::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string Int::GetInputCode(std::string var) {
  std::string ret;
  ret += "// Int::GetInputCode\n";
  if (m_pointer == 0) {
    ret += "scanf(\"%d\", &"+var+");\n";
  } else if (m_pointer == 1) {
    ret += var + "=(int*)malloc(sizeof(int)*helium_size);\n";
  } else {
    assert(false && "More than int**?");
  }
  return ret;
}

std::string Int::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Int::GetOutputCode\n";
  ret += "printf(\"%d\\n\", " + var + ");\n";
  return ret;
}

std::string Int::GetTestInput() {
  std::string ret;
  if (m_pointer == 0) {
    int a = utils::rand_int(0, 100);
    ret += std::to_string(a);
  } else if (m_pointer == 1) {
    int size = utils::rand_int(0, 5);
    ret += std::to_string(size);
    for (int i=0;i<size;i++) {
      int a = utils::rand_int(0,100);
      ret += " " + std::to_string(a);
    }
  } else {
    assert(false);
  }
  return ret;
}


std::string Char::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Char::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string Char::GetInputCode(std::string var) {
  std::string ret;
  ret += "// Char::GetInputCode\n";
  if (m_pointer == 0) {
    if (m_dimension == 0) {
      ret += "scanf(\"%c\", &"+var+");\n";
    } else if (m_dimension == 1) {
      // TODO
    } else if (m_dimension == 2) {
      // TODO
    } else {
      assert(false && "char [][][]");
    }
  } else if (m_pointer == 1) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    ret += "scanf(\"%d\", &helium_size);\n";
    // ret += "char* "+var+";\n";
    // ret += "if (helium_size == 0) {\n";
    // ret += "  " + var + " = NULL;\n";
    // ret += "} else {\n";
    // ret += "  " + var + " = (char*)malloc(sizeof(char)*helium_size);\n";
    // NOTE: we don't need to have a branch to tell whether helium_size equals to 0.
    // malloc will return NULL.
    // FIXME but in this case, scanf will crash. It may not matter.
    ret += var + " = (char*)malloc(sizeof(char)*helium_size);\n";
    ret += "scanf(\"%s\", "+var+");\n}"; // FIXME this should be less than helium_size? Or just let the oracle do the trick
  } else if (m_pointer == 2) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    /**
     * char **a;
     * The input code should be:
     * char **a;
     * a = (char**)malloc(sizeof(char*)*helium_size);
     * for (int i=0;i<helium_size;i++) {
     *  int helium_size;
     *  scanf(helium_size);
     *  a[i] = (char*)malloc(sizeof(char*)*helium_size);
     * }
     */
    ret += "scanf(\"%d\", &helium_size);\n";
    ret += var + " = (char**)malloc(sizeof(char*)*helium_size);\n";
    ret += "for (int i=0;i<helium_size;i++) {\n";
    ret += "  int helium_size;\n";
    ret += "  scanf(\"%d\", &helium_size);\n";
    ret += "  " + var + "[i] = (char*)malloc(sizeof(char)*helium_size);\n";
    ret += "  scanf(\"%s\", "+var+"[i]);\n}";
  } else {
    assert(false && "char ***");
  }
  // if (m_pointer == 0 && m_dims.size() == 0) {
  //   // ret += m_raw + " " + var + ";\n";
  // } else if (m_pointer == 1 || m_dims.size() == 1) {
  //   ret += "scanf(\"%d\", &helium_size);\n";
  //   ret += "char* "+var+";\n";
  //   ret += "if (helium_size == 0) {\n";
  //   ret += "  " + var + " = NULL;\n";
  //   ret += "} else {\n";
  //   ret += "  " + var + " = (char*)malloc(sizeof(char)*helium_size);\n";
  //   ret += "  scanf(\"%s\", "+var+");\n}";
  // }
  return ret;
}

/**
 * Char input generation is a little tricky.
 * We need to intentionally insert some special characters.
 * -, :, ", ', #, $
 */
std::string Char::GetTestInput() {
  std::string ret;
  if (m_pointer == 0) {
    if (m_dimension == 0) {
      ret += utils::rand_char();
    } else if (m_dimension == 1) {
      // TODO
    } else if (m_dimension == 2) {
      // TODO
    } else {
      assert(false);
    }
  } else if (m_pointer == 1) {
    int size = utils::rand_int(0,5);
    ret += std::to_string(size);
    for (int i=0;i<size;i++) {
      ret += " " + utils::rand_str(utils::rand_int(0, 10));
    }
  } else if (m_pointer == 2) {
    // TODO
    int size = utils::rand_int(0, 5);
    ret += std::to_string(size);
    for (int i=0;i<size;i++) {
      int size2 = utils::rand_int(0, 5);
      ret += " " + std::to_string(size);
      for (int j=0;j<size2;j++) {
        ret += utils::rand_str(utils::rand_int(0, 10));
      }
    }
  } else {
    assert(false);
  }
  return ret;
}

/**
 * TODO dimension and pointer
 */
std::string Char::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Char::GetOutputCode\n";
  // ret += "printf(\"%d\\n\", " + var + ");\n";
  if (m_pointer == 0) {
    if (m_dimension == 0) {
      ret += "printf(\"%c\\n\", " + var + ");\n";
    } else if (m_dimension == 1) {
      // TODO
      // ret += "// HELIUM_TODO char[]\n";
      ret += "printf(\"sizeof(" + var + ") = %d\\n\", sizeof(" + var + "));\n";
      ret += "printf(\"strlen(" + var + ") = %d\\n\", strlen(" + var + "));\n";
    } else if (m_dimension == 2) {
      // TODO
      ret += "// HELIUM_TODO char[][]\n";
    } else {
      assert(false && "char [][][]");
    }
  } else if (m_pointer == 1) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    ret += "printf(\"" + var + " = %s\\n\", " + var + ");\n";
    ret += "printf(\"strlen(" + var + ") = %d\\n\", strlen(" + var + "));\n";
  } else if (m_pointer == 2) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    // TODO the size of the buffer?
    ret += "printf(\"" + var + "[0] = " +  "%s\\n\", " + var + "[0]);\n";
    ret += "printf(\"sizeof(" + var + ") = %d\\n\", sizeof("+var+"));\n";
  }
  return ret;
}




std::string Float::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Float::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string Float::GetInputCode(std::string var) {
  std::string ret;
  ret += "// Float::GetInputCode\n";
  // a double should have pointer level just 0?
  assert(m_pointer == 0);
  ret += "scanf(\"%lf\", " + var + ");\n}";
  return ret;
}

std::string Float::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Float::GetOutputCode\n";
  ret += "printf(\"%f\\n\", " + var + ");\n";
  return ret;
}


std::string Float::GetTestInput() {
  std::string ret;
  return ret;
}

std::string Void::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Void::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string Void::GetInputCode(std::string var) {
  std::string ret;
  ret += "// Void::GetInputCode\n";
  var = "No use";
  assert(m_pointer > 0 && "A void should be a pointer");
  // what to be generate here?
  assert(false && "I'm not clear whether to generate input for a void*");
  return ret;
}
std::string Void::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Void::GetOutputCode\n";
  // TODO check if it is NULL
  ret += "printf(\"%d\\n\", " + var + ");\n";
  return ret;
}

std::string Void::GetTestInput() {
  std::string ret;
  return ret;
}


std::string Bool::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// Bool::GetDeclCode\n";
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string Bool::GetInputCode(std::string var) {
  std::string ret;
  ret += "// Bool::GetInputCode\n";
  ret += "scanf(\"%d\", &" + var + ");\n";
  return ret;
}

std::string Bool::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Bool::GetOutputCode\n";
  ret += "printf(\"%d\\n\", " + var + ");\n";
  return ret;
}

std::string Bool::GetTestInput() {
  std::string ret;
  return ret;
}
