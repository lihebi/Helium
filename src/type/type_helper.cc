#include "type_helper.h"
#include "type_common.h"

using namespace utils;

LoopHelper* LoopHelper::m_instance = NULL;


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
  // TODO Hotfix for bool _Bool
  bool b1 = search_and_remove(name, boost::regex("\\bbool\\b")) ? 1 : 0;
  bool b2 = search_and_remove(name, boost::regex("\\b_Bool\\b")) ? 1 : 0;
  specifier.is_bool = (b1 == 1 || b2 == 1);
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

/**
 * Usage:
 * - get_scanf_code("%c", "&" + var);
 */
std::string get_scanf_code(std::string format, std::string var) {
  var = var + LoopHelper::Instance()->GetSuffix();
  // std::cout << "getting scanf code. Current level: " << LoopHelper::Instance()->GetLevel() << "\n";
  return "scanf(\"" + format + "\", " + var + ");\n";
}

/**
 * Do not add suffix
 */
std::string get_scanf_code_raw(std::string format, std::string var) {
  return "scanf(\"" + format + "\", " + var + ");\n";
}

std::string get_malloc_code(std::string var, std::string type, std::string size) {
  var = var + LoopHelper::Instance()->GetSuffix();
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

std::string get_sizeof_printf_code(std::string var) {
  std::string ret;
  std::string format_var = var + LoopHelper::Instance()->GetFormatSuffix();
  std::string value_var = var + LoopHelper::Instance()->GetSuffix();
  ret += "printf(\"int_" + format_var + ".size = %d\\n\", ";
  ret += LoopHelper::Instance()->GetIndexVar();
  ret += "sizeof(" + value_var + "));\n" + flush_output;
  return ret;
}
std::string get_strlen_printf_code(std::string var) {
  std::string ret;
  std::string format_var = var + LoopHelper::Instance()->GetFormatSuffix();
  std::string value_var = var + LoopHelper::Instance()->GetSuffix();
  ret += "printf(\"int_" + format_var + ".strlen = %ld\\n\", ";
  ret += LoopHelper::Instance()->GetIndexVar();
  ret += "strlen(" + value_var + "));\n" + flush_output;
  return ret;
}

std::string get_addr_printf_code(std::string var) {
  std::string ret;
  std::string format_var = var + LoopHelper::Instance()->GetFormatSuffix();
  std::string value_var = var + LoopHelper::Instance()->GetSuffix();
  ret += "printf(\"addr_" + format_var + " = %p\\n\", ";
  ret += LoopHelper::Instance()->GetIndexVar();
  ret += "(void*)" + value_var + ");\n" + flush_output;
  return ret;
}

/**
 * print it is equal to null, even based on the parameters, not the runtime value.
 */
std::string get_isnull_printf_code(std::string var, bool is_null) {
  std::string ret;
  std::string format_var = var + LoopHelper::Instance()->GetFormatSuffix();
  ret += "printf(\"isnull_" + format_var + " = %d\\n\", ";
  ret += LoopHelper::Instance()->GetIndexVar();
  if (is_null) {
    ret += "1";
  } else {
    ret += "0";
  }
  ret += ");\n" + flush_output;
  return ret;
}

/**
 * branch construction for NULL of a variable
 */
std::string get_check_null(std::string var, std::string true_branch, std::string false_branch) {
  var = var + LoopHelper::Instance()->GetSuffix();
  std::string ret;
  ret += "if (" + var + " == NULL) {\n";
  ret += true_branch;
  ret += "} else {\n";
  ret += false_branch;
  ret += "}\n";
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


/**
 * Heap size: 
 */
std::string LoopHelper::GetHeliumHeapCode(std::string var, std::string body) {
  std::string ret;
  // std::cout << "Current levl: " << m_level << "\n";
  // use the last suffix ...
  std::string value_var = var + LoopHelper::Instance()->GetLastSuffix();
  std::string format_var = var + LoopHelper::Instance()->GetLastFormatSuffix();
  std::string idxvar = GetCurrentIndexVar();
  ret += "helium_heap_target_size = -1;\n";
  ret += "for (int " + idxvar + "=0;" + idxvar + "<helium_heap_top;" + idxvar + "++) {\n";
  ret += "  if (" + value_var + " == helium_heap_addr[" + idxvar + "]) {\n";
  ret += "    helium_heap_target_size = helium_heap_size[" + idxvar + "];\n";
  ret += "    break;\n";
  ret += "  }\n";
  ret += "}\n";
  ret += "if (helium_heap_target_size != -1) {\n";
  ret += "  printf(\"int_" + format_var + "_heapsize = %d\\n\", ";
  ret += LoopHelper::Instance()->GetLastIndexVar();
  ret += "helium_heap_target_size);\n";
  ret += flush_output;
  ret += "  for (int " + idxvar + "=0,helium_heap_target_size_tmp=helium_heap_target_size;"
    + idxvar + "<helium_heap_target_size_tmp;" + idxvar + "++) {\n";
  ret += "// HeliumHeapCode body\n";
  ret += body;
  ret += " }\n";
  ret += "}\n";
  return ret;
}

/**
 * Heap Address Size recorder
 */
std::string LoopHelper::GetSuffix() {
  std::string ret;
  for (int i=1;i<=m_level;i++) {
    ret += "[" + std::string(i, 'i') + "]";
  }
  return ret;
}

// suffix to be used in suffix string
std::string LoopHelper::GetFormatSuffix() {
  std::string ret;
  for (int i=1;i<=m_level;i++) {
    ret += "[%d]";
  }
  return ret;
}
std::string LoopHelper::GetLastFormatSuffix() {
  std::string ret;
  for (int i=1;i<=m_level-1;i++) {
    ret += "[%d]";
  }
  return ret;
}

/**
 * Use the m_level-1 as level
 */
std::string LoopHelper::GetLastSuffix() {
  std::string ret;
  for (int i=1;i<m_level;i++) {
    ret += "[" + std::string(i, 'i') + "]";
  }
  return ret;
}

std::string LoopHelper::GetIndexVar() {
  std::string ret;
  for (int i=1;i<=m_level;i++) {
    ret += std::string(i,'i') + ", ";
  }
  return ret;
}
std::string LoopHelper::GetLastIndexVar() {
  std::string ret;
  for (int i=1;i<=m_level-1;i++) {
    ret += std::string(i,'i') + ", ";
  }
  return ret;
}

/**
 * loop by helium_size. The index is "i"
 */
std::string LoopHelper::GetHeliumSizeLoop(std::string body) {
  std::string ret;
  std::string var = GetCurrentIndexVar();
  // Use helium_size_tmp because the body might overwrite helium_size
  ret += "for (int " + var + "=0,helium_size_tmp=helium_size;" + var + "<helium_size_tmp;" + var + "++) {\n";
  ret += body;
  ret += "}\n";
  return ret;
}
