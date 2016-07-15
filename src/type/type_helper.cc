#include "type_helper.h"
#include "type_common.h"

using namespace utils;

namespace type {

  bool search_and_remove(std::string &s, boost::regex reg) {
    if (boost::regex_search(s, reg)) {
      s = boost::regex_replace<boost::regex_traits<char>, char>(s, reg, "");
      return true;
    }
    return false;
  }

  int count_and_remove(std::string &s, char c) {
    int count = std::count(s.begin(), s.end(), c);
    if (count) {
      s.erase(std::remove(s.begin(), s.end(), c), s.end());
    }
    return count;
  }

  void fill_storage_specifier(std::string& name, struct storage_specifier& specifier) {
    specifier.is_auto     = search_and_remove(name, boost::regex("\\bauto\\b"))     ? 1 : 0;
    specifier.is_register = search_and_remove(name, boost::regex("\\bregister\\b")) ? 1 : 0;
    specifier.is_static   = search_and_remove(name, boost::regex("\\bstatic\\b"))   ? 1 : 0;
    specifier.is_extern   = search_and_remove(name, boost::regex("\\bextern\\b"))   ? 1 : 0;
    // specifier.is_typedef     = search_and_remove(name, typedef_regex)     ? 1 : 0;
  }

  void fill_type_specifier(std::string& name, struct type_specifier& specifier) {
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
  void fill_type_qualifier(std::string& name, struct type_qualifier& qualifier) {
    qualifier.is_const    = search_and_remove(name, boost::regex("\\bconst\\b"))    ? 1 : 0;
    qualifier.is_volatile = search_and_remove(name, boost::regex("\\bvolatile\\b")) ? 1 : 0;
  }

  void fill_struct_specifier(std::string& name, struct struct_specifier& specifier) {
    specifier.is_struct = search_and_remove(name, boost::regex("\\bstruct\\b")) ? 1 : 0;
    specifier.is_enum   = search_and_remove(name, boost::regex("\\benum\\b"))   ? 1 : 0;
    specifier.is_union  = search_and_remove(name, boost::regex("\\bunion\\b"))  ? 1 : 0;
  }



  /********************************
   * Helper functions
   *******************************/
  const std::string flush_output = "fflush(stdout);\n";
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

  std::string get_sizeof_output(std::string var) {
    return "printf(\"Od_sizeof(" + var + ") = %d\\n\", sizeof(" + var + "));\n" + flush_output;
  }
  std::string get_strlen_output(std::string var) {
    return "printf(\"Od_strlen(" + var + ") = %d\\n\", strlen(" + var + "));\n" + flush_output;
  }
  std::string get_addr_output(std::string var) {
    return "printf(\"Op_addr(" + var + ") = %p\\n\", (void*)" + var + ");\n" + flush_output;
    // if (Config::Instance()->GetString("use-address") == "true") {
    //   return "printf(\"Op_addr(" + var + ") = %p\\n\", (void*)" + var + ");\n" + flush_output;
    // } else {
    //   return "";
    // }
  }
  std::string get_addr_input(std::string var) {
    return "printf(\"Ip_addr(" + var + ") = %p\\n\", (void*)" + var + ");\n" + flush_output;
    // if (Config::Instance()->GetString("use-address") == "true") {
    //   return "printf(\"Ip_addr(" + var + ") = %p\\n\", (void*)" + var + ");\n" + flush_output;
    // } else {
    //   return "";
    // }
  }
  std::string get_check_null_if(std::string var) {
    return "if (" + var + " == NULL) {\n";
  }
  std::string get_check_null_else() {
    return "} else {\n";
  }
  std::string get_check_null_fi() {
    return "}\n";
  }

  /**
   * print it is equal to null, even based on the parameters, not the runtime value.
   */
  std::string get_null_output(std::string var, bool is_null) {
    if (is_null) {
      return "printf(\"On_" + var + " = NULL\\n\");\n" + flush_output;
    } else {
      return "printf(\"On_" + var + " = !NULL\\n\");\n" + flush_output;
    }
    // if (Config::Instance()->GetString("null-output") == "true") {
    //   if (is_null) {
    //     return "printf(\"On_" + var + " == NULL\\n\");\n" + flush_output;
    //   } else {
    //     return "printf(\"On_" + var + " = !NULL\\n\");\n" + flush_output;
    //   }
    // } else {
    //   return "";
    // }
  }

  /**
   * branch construction for NULL of a variable
   */
  std::string get_check_null(std::string var, std::string true_branch, std::string false_branch) {
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

  /**
   * Get the raw ID of the type
   */
  std::string get_id(std::string raw_type) {
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

  bool is_primitive(std::string s) {
    if (s.find('[') != std::string::npos) {
      s = s.substr(0, s.find('['));
    }
    count_and_remove(s, '*');
    struct storage_specifier storage_specifier;
    struct type_specifier type_specifier;
    struct type_qualifier type_qualifier;
    struct struct_specifier struct_specifier;
    fill_storage_specifier(s, storage_specifier);
    fill_type_specifier(s, type_specifier);
    fill_type_qualifier(s, type_qualifier);
    fill_struct_specifier(s, struct_specifier);
    trim(s);
    if (s.empty()) return true;
    return false;
  }

}
