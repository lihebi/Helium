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

std::string get_sizeof_output(std::string var);
std::string get_strlen_output(std::string var);
std::string get_addr_output(std::string var);
std::string get_addr_input(std::string var);
std::string get_check_null_if(std::string var);
std::string get_check_null_else();
std::string get_check_null_fi();
std::string get_null_output(std::string var, bool is_null);
std::string get_check_null(std::string var, std::string true_branch, std::string false_branch);

std::string get_helium_size_branch(std::string true_branch, std::string false_branch);
std::string get_helium_size_loop(std::string body);
std::string get_str_input_code(std::string name);

std::string get_id(std::string raw_type);

bool is_primitive(std::string s);


#endif /* TYPE_HELPER_H */
