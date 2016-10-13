#ifndef TYPE_HELPER_H
#define TYPE_HELPER_H

#include <string>
#include <boost/regex.hpp>

struct storage_specifier {
  unsigned int is_auto     : 1;
  unsigned int is_register : 1;
  unsigned int is_static   : 1;
  unsigned int is_extern   : 1;
  unsigned int is_typedef  : 1; // not used
};
struct type_specifier {
  unsigned int is_void     : 1;
  unsigned int is_char     : 1;
  unsigned int is_short    : 1;
  unsigned int is_int      : 1;
  unsigned int is_long     : 1; // do not support long long
  unsigned int is_float    : 1;
  unsigned int is_double   : 1;
  unsigned int is_signed   : 1;
  unsigned int is_unsigned : 1;
  unsigned int is_bool     : 1; // not in C standard
};
struct type_qualifier {
  unsigned int is_const    : 1;
  unsigned int is_volatile : 1;
};
struct struct_specifier {
  unsigned int is_struct   : 1;
  unsigned int is_union    : 1;
  unsigned int is_enum     : 1;
};

bool search_and_remove(std::string &s, boost::regex reg);
int count_and_remove(std::string &s, char c);
void fill_storage_specifier(std::string& name, struct storage_specifier& specifier);
void fill_type_specifier(std::string& name, struct type_specifier& specifier);
void fill_type_qualifier(std::string& name, struct type_qualifier& qualifier);
void fill_struct_specifier(std::string& name, struct struct_specifier& specifier);

extern const std::string flush_output;

std::string get_scanf_code(std::string format, std::string var);
std::string get_malloc_code(std::string var, std::string type, std::string size);

std::string get_sizeof_printf_code(std::string var);
std::string get_strlen_printf_code(std::string var);


std::string get_addr_printf_code(std::string var);
std::string get_check_null_if(std::string var);
std::string get_check_null_else();
std::string get_check_null_fi();
std::string get_isnull_printf_code(std::string var, bool is_null);
std::string get_check_null(std::string var, std::string true_branch, std::string false_branch);

std::string get_helium_size_branch(std::string true_branch, std::string false_branch);

std::string get_id(std::string raw_type);

bool is_primitive(std::string s);

class LoopHelper {
public:
  static LoopHelper* Instance() {
    if (!m_instance) {
      m_instance = new LoopHelper();
    }
    return m_instance;
  }
  std::string GetCurrentIndexVar() {
    return std::string(m_level, 'i');
  }
  void IncLevel() {
    m_level++;
  }
  void DecLevel() {
    m_level--;
  }
  /**
   * loop by helium_size. The index is "i"
   */
  std::string GetHeliumSizeLoop(std::string body) {
    std::string ret;
    std::string var = GetCurrentIndexVar();
    // Use helium_size_tmp because the body might overwrite helium_size
    ret += "for (int " + var + "=0,helium_size_tmp=helium_size;" + var + "<helium_size_tmp;" + var + "++) {\n";
    ret += body;
    ret += "}\n";
    return ret;
  }
  /**
   * Heap Address Size recorder
   */
  std::string GetHeliumHeapCode(std::string var, std::string body) {
    std::string ret;
    std::string idxvar = GetCurrentIndexVar();
    ret += "helium_heap_target_size = -1;\n";
    ret += "for (int " + idxvar + "=0;" + idxvar + "<helium_heap_top;" + idxvar + "++) {\n";
    ret += "  if (" + var + " == helium_heap_addr[" + idxvar + "]) {\n";
    ret += "    helium_heap_target_size = helium_heap_size[" + idxvar + "];\n";
    ret += "    break;\n";
    ret += "  }\n";
    ret += "}\n";
    ret += "if (helium_heap_target_size != -1) {\n";
    ret += "  printf(\"int_" + var + "_heapsize = %d\\n\", helium_heap_target_size);\n";
    ret += flush_output;
    ret += "  for (int " + idxvar + "=0,helium_heap_target_size_tmp=helium_heap_target_size;"
      + idxvar + "<helium_heap_target_size_tmp;" + idxvar + "++) {\n";
    ret += body;
    ret += " }\n";
    ret += "}";
    return ret;
  }
private:
  static LoopHelper *m_instance;
  int m_level=1;
};


#endif /* TYPE_HELPER_H */
