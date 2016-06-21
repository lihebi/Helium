#include "new_type.h"
#include "utils.h"
#include "resolver.h"
#include <iostream>
#include "config.h"
#include "options.h"

#include <gtest/gtest.h>

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
 * The return value should be free-d by user.
 */
TestInput* NewType::GetTestInputSpec(std::string var) {
  TestInput *ret = new TestInput(this, var);
  ret->SetRaw(GetTestInput());
  // std::cout << ret->GetRaw()  << "\n";
  // std::cout << GetTestInput()  << "\n";
  // getchar();
  return ret;
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
  // print_trace("NewTypeFactory::CreateType: " +  raw);
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
    // TODO FIXME should I just return NULL?
    // But anyway the SnippetRegistry is DEPRECATED
    // return NULL;
    return new LocalNewType(raw, dims);
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
  // in another word, should not return NULL
  // FIXME TODO handle the case where the type is not created correctly (NULL)
  // It should be in model.cc, in Decl class
  assert(false);
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

const std::string flush_output = "fflush(stdout);\n";


/********************************
 * Models
 *******************************/

/********************************
 * Helper functions
 *******************************/
/**
 * Usage:
 * - get_scanf_code("%c", "&" + var);
 */
std::string get_scanf_code(std::string format, std::string var) {
  return "scanf(\"" + format + "\", " + var + ");\n";
}

std::string get_malloc_code(std::string var, std::string type, std::string size) {
  return var + " = (" + type + "*)malloc(sizeof(" + type + ")*" + size + ");\n";
}

/**
 * For the printfs, the prefix will tell:
 * - I/O
 * - type:
 *   + d: int
 *   + p: address, hex
 *   + n: NULL, !NULL
 *   + c: char
 *   + x: an arbitrary string
 */

static std::string get_sizeof_output(std::string var) {
  return "printf(\"Od_sizeof(" + var + ") = %d\\n\", sizeof(" + var + "));\n" + flush_output;
}
static std::string get_strlen_output(std::string var) {
  return "printf(\"Od_strlen(" + var + ") = %d\\n\", strlen(" + var + "));\n" + flush_output;
}
static std::string get_addr_output(std::string var) {
  if (Config::Instance()->GetString("use-address") == "true") {
    return "printf(\"Op_addr(" + var + ") = %p\\n\", (void*)" + var + ");\n" + flush_output;
  } else {
    return "";
  }
}
static std::string get_addr_input(std::string var) {
  if (Config::Instance()->GetString("use-address") == "true") {
    return "printf(\"Ip_addr(" + var + ") = %p\\n\", (void*)" + var + ");\n" + flush_output;
  } else {
    return "";
  }
}
static std::string get_check_null_if(std::string var) {
  return "if (" + var + " != NULL) {\n";
}
static std::string get_check_null_else() {
  return "} else {\n";
}
static std::string get_check_null_fi() {
  return "}\n";
}
static std::string get_null_output(std::string var, bool is_null) {
  if (Config::Instance()->GetString("null-output") == "true") {
    if (is_null) {
      return "printf(\"On_" + var + " == NULL\\n\");\n" + flush_output;
    } else {
      return "printf(\"On_" + var + " = !NULL\\n\");\n" + flush_output;
    }
  } else {
    return "";
  }
}

/**
 * branch construction for NULL of a variable
 */
static std::string get_check_null(std::string var, std::string true_branch, std::string false_branch) {
  std::string ret;
  ret += get_check_null_if(var);
  ret += true_branch;
  ret += get_check_null_else();
  ret += false_branch;
  ret += get_check_null_fi();
  return ret;
}

/**
 * Check whether helium_size is 0
 */
std::string get_helium_size_branch(std::string true_branch, std::string false_branch) {
  std::string ret;
  ret += "if (helium_size == 0) {\n";
  ret += true_branch;
  ret += "} else {\n";
  ret += false_branch;
  ret += "}\n";
  return ret;
}

/**
 * loop by helium_size. The index is "i"
 */
std::string get_helium_size_loop(std::string body) {
  std::string ret;
  ret += "for (int i=0;i<helium_size;i++) {\n";
  ret += body;
  ret += "}\n";
  return ret;
}

/**
 * generate code for a string scanf
 */
std::string get_str_input_code(std::string name) {
  std::string ret;
  ret += get_scanf_code("%d", "&helium_size");
  ret += get_malloc_code(name, "char", "helium_size");
  ret += get_helium_size_branch(name + " = NULL;\n",
                                get_scanf_code("%s", name)
                                );
  return ret;
}


/********************************
 * Models
 *******************************/



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
    ret += get_scanf_code("%d", "&"+var);
  } else if (m_pointer == 1) {
    ret += get_malloc_code(var, "int", "helium_size");
  } else {
    assert(false && "More than int**?");
  }
  return ret;
}

std::string Int::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Int::GetOutputCode\n";
  // TODO extract this into a helper function
  ret += "printf(\"Od_" + var + " = %d\\n\", " + var + ");\n" + flush_output;
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
      ret += get_scanf_code("%c", "&" + var);
    } else if (m_dimension == 1) {
      // TODO
      ret += "// HELIUM_TODO char[], should we init this?\n";
    } else if (m_dimension == 2) {
      // TODO
      ret += "// HELIUM_TODO char[][]\n";
    } else {
      assert(false && "char [][][]");
    }
  } else if (m_pointer == 1) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    // ret += get_scanf_code("%d", "&helium_size");
    // ret += get_malloc_code(var, "char", "helium_size");
    ret += get_str_input_code(var);
    ret += get_addr_input(var);
    // FIXME this should be less than helium_size? Or just let the oracle do the trick
    // ret += get_scanf_code("%s", var);
  } else if (m_pointer == 2) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    ret += get_scanf_code("%d", "&helium_size");
    ret += get_malloc_code(var, "char*", "helium_size");
    ret += get_addr_input(var);
    ret += get_helium_size_loop("int helium_size;\n" + get_str_input_code(var + "[i]"));
  } else {
    assert(false && "char ***");
  }
  return ret;
}

/**
 * DEPRECATED Use GetTestInputSpec instead.
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
    // int size = utils::rand_int(0,5);
    // ret += std::to_string(size);
    // for (int i=0;i<size;i++) {
    //   ret += " " + utils::rand_str(utils::rand_int(0, 10));
    // }
    int size = utils::rand_int(0, 3000);
    ret += " " + std::to_string(size) + " ";
    if (size != 0) {
      ret += utils::rand_str(utils::rand_int(1, size));
    }
  } else if (m_pointer == 2) {
    // TODO
    int size = utils::rand_int(0, 5);
    ret += std::to_string(size);
    for (int i=0;i<size;i++) {
      int size2 = utils::rand_int(0, 3000);
      ret += " " + std::to_string(size2) + " ";
      if (size2 != 0) {
        ret += utils::rand_str(utils::rand_int(1, size2));
      }
    }
  } else {
    assert(false);
  }
  return ret;
}

TestInput* Char::GetTestInputSpec(std::string var) {
  std::string raw;
  std::vector<int> strlens;
  if (m_pointer == 0) {
    if (m_dimension == 0) {
      raw += utils::rand_char();
    } else if (m_dimension == 1) {
      // TODO
    } else if (m_dimension == 2) {
      // TODO
    } else {
      assert(false);
    }
  } else if (m_pointer == 1) {
    // buffer size
    int size = utils::rand_int(0, Config::Instance()->GetInt("max-strlen"));
    raw += std::to_string(size) + " ";
    int len = 0; // strlen
    if (size != 0) {
      std::string str = utils::rand_str(utils::rand_int(1, size));
      raw += str;
      len = str.length();
    }
    strlens.push_back(len);
    // for (int i=0;i<size;i++) {
    //   raw += " " + utils::rand_str(utils::rand_int(0, size));
    // }
  } else if (m_pointer == 2) {
    // TODO
    int size = utils::rand_int(0, 5);
    raw += std::to_string(size);
    for (int i=0;i<size;i++) {
      int size2 = utils::rand_int(0, Config::Instance()->GetInt("max-strlen"));
      raw += " " + std::to_string(size2) + " ";
      // for (int j=0;j<size2;j++) {
      //   raw += utils::rand_str(utils::rand_int(0, 10)) + " ";
      // }
      int len = 0;
      if (size2 != 0) {
        std::string str = utils::rand_str(utils::rand_int(1, size2));
        raw += str;
        len = str.length();
      }
      strlens.push_back(len);
    }
  } else {
    assert(false);
  }
  CharTestInput *ret = new CharTestInput(this, var);
  ret->SetRaw(raw);
  ret->SetStrlen(strlens);
  return ret;
}


/**
 * TODO dimension and pointer
 */
std::string Char::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Char::GetOutputCode\n";
  if (m_pointer == 0) {
    if (m_dimension == 0) {
      ret += "printf(\"Oc_" + var + " = %c\\n\", " + var + ");\n" + flush_output;
    } else if (m_dimension == 1) {
      // TODO
      ret += get_sizeof_output(var);
      // FIXME for a char[], only output the sizeof the buffer.
      // this is because I got too many not important invariants
      // ret += get_strlen_output(var);
      ret += get_addr_output(var);
    } else if (m_dimension == 2) {
      // TODO
      ret += "// HELIUM_TODO char[][]\n";
    } else {
      assert(false && "char [][][]");
    }
  } else if (m_pointer == 1) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    // FIXME For a char pointer, do not output the size.
    // ret += get_sizeof_output(var);
    ret += get_strlen_output(var);
    ret += get_addr_output(var);
  } else if (m_pointer == 2) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    // TODO the size of the buffer?
    ret += get_check_null(var,
                          get_null_output(var, false)
                          + get_addr_output(var)
                          + get_check_null("*"+var,
                                           get_null_output("*"+var, false)
                                           + get_strlen_output("*"+var)
                                           + get_addr_output("*"+var),
                                           get_null_output("*"+var, true)
                                           ),
                          get_null_output(var, true)
                          );
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
  ret += "printf(\"Of_" + var + " = %f\\n\", " + var + ");\n" + flush_output;
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
  ret += "printf(\"%d\\n\", " + var + ");\n" = flush_output;
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
  ret += "printf(\"%d\\n\", " + var + ");\n" + flush_output;
  return ret;
}

std::string Bool::GetTestInput() {
  std::string ret;
  return ret;
}



/********************************
 * TestInputSpec
 *******************************/

std::string CharTestInput::dump() {
  std::string ret;
  ret += m_type->ToString() + " " + m_var + "\n";
  ret += "size: " + std::to_string(m_strlens.size()) + ", ";
  ret += "strlens:";
  for (int len : m_strlens) {
    ret += " " + std::to_string(len);
  }
  return ret;
}

/**
 * Input ToString function is used for record information.
 * E.g. the size of the buffer, the strlen.
 */
std::string CharTestInput::ToString() {
  std::string ret;
  int size = m_strlens.size();
  ret += "Id_" + m_var + ".size() = " + std::to_string(size) + "\n";
  for (int i=0;i<size;++i) {
    ret += "Id_strlen(" + m_var + "[" + std::to_string(i) + "]) = " + std::to_string(m_strlens[i]) + "\n";
  }
  return ret;
}


/********************************
 * ArgCV
 *******************************/

std::vector<std::pair<TestInput*, TestInput*> > ArgCV::GetTestInputSpec(int number) {
    std::vector<std::pair<TestInput*, TestInput*> > ret;
    for (int i=0;i<number;i++) {
      ret.push_back(GetTestInputSpec());
    }
    return ret;
}

std::pair<TestInput*, TestInput*> ArgCV::GetTestInputSpec() {
  ArgVTestInput *argv_input = new ArgVTestInput(NULL, "argv");
  // according to spec, generate the raw
  std::vector<std::string> components;
  std::string argv_spec;
  components.push_back("helium_program");
  for (char c : m_bool_opt) {
    if (utils::rand_bool()) {
      components.push_back("-" + std::string(1, c));
      argv_spec += "Ix_argv:" + std::string(1, c) + " = true\n";
    } else {
      argv_spec += "Ix_argv:" + std::string(1, c) + " = false\n";
    }
  }
  for (char c : m_str_opt) {
    if (utils::rand_bool()) {
      components.push_back("-" + std::string(1, c));
      // the string for the argument
      int size = utils::rand_int(1, Config::Instance()->GetInt("max-strlen"));
      std::string str = utils::rand_str(utils::rand_int(1, size));
      components.push_back(str);
      argv_spec += "Ix_argv:" + std::string(1, c) + " = true\n";
      argv_spec += "Id_strlen(argv:" + std::string(1, c) + ") = " + std::to_string(str.length())+ "\n";
    }
  }
  // add 0-3 more random staff
  int random_staff = utils::rand_int(0, 3);
  for (int i=0;i<random_staff;i++) {
    int size = utils::rand_int(1, Config::Instance()->GetInt("max-strlen"));
    std::string str = utils::rand_str(utils::rand_int(1, size));
    components.push_back(str);
    argv_spec += "Id_strlen(argv:r" + std::to_string(i) + ") = " + std::to_string(str.length()) + "\n";
  }
  std::string raw;
  raw += std::to_string(components.size()) + " ";
  for (std::string &s : components) {
    raw += std::to_string(s.length()+1) + " ";
    raw += s + " ";
  }
  TestInput *argc_input = new TestInput(NULL, "argc");
  int size = components.size();
  argc_input->SetRaw(std::to_string(size));
  argv_input->SetRaw(raw);
  argv_input->SetSpec(argv_spec);
  return {argc_input, argv_input};
}

/**
 * Disable because the config file seems not loaded correctly, for max-strlen
 */
TEST(NewTypeTestCase, DISABLED_ArgCVTest) {
  ArgCV argcv;
  argcv.SetOpt("achtvf:");
  std::pair<TestInput*, TestInput*> inputs = argcv.GetTestInputSpec();
  // inputs.first->ToString();
  // inputs.first->GetRaw();
  // inputs.second->ToString();
  // inputs.second->GetRaw();

  std::cout << inputs.first->ToString() << "\n";
  std::cout << inputs.first->GetRaw() << "\n";
  std::cout << inputs.second->ToString() << "\n";
  std::cout << inputs.second->GetRaw() << "\n";
}
