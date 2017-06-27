#include "helium/type/io_helper.h"

IOHelper* IOHelper::m_instance = NULL;


std::string FileIOHelper::GetIntCode() {
  return R"prefix(
int get_int() {
  int ret;
  fscanf(int_input_fp, "%d", &ret);
  return ret;
}
void input_int(int *a) {
  *a = get_int();
}
void input_int_name(int *a, char *name) {
  *a = get_int();
  fprintf(output_fp, "input: int %s=%d\n", name, *a);
}

void output_int(int *a, char *name) {
  fprintf(output_fp, "output: int %s=%d\n", name, *a);
}

char get_char() {
  char ret;
  fscanf(char_input_fp, "%c", &ret);
  return ret;
}
void input_char(char *c) {
  *c = get_char();
}
void input_char_name(char *c, char *name) {
  *c = get_char();
  fprintf(output_fp, "input: char %s=%c\n", name, *c);
}

void output_char(char *c, char *name) {
  fprintf(output_fp, "output: char %s=%c\n", name, *c);
}


bool get_bool() {
  int tmp;
  fscanf(bool_input_fp, "%d", &tmp);
  return tmp==1;
}
void input_bool(bool *b) {
  *b = get_bool();
}
void input_bool_name(bool *b, char *name) {
  *b = get_bool();
  fprintf(output_fp, "input: bool %s=%d\n", name, *b);
}
void output_bool(bool *b, char *name) {
  fprintf(output_fp, "output: bool %s=%d\n", name, *b);
}


)prefix";
}

std::string FileIOHelper::GetGlobalFilePointers() {
  return R"prefix(
// global file pointers
FILE *int_input_fp;
FILE *char_input_fp;
FILE *bool_input_fp;
FILE *output_fp;
)prefix";
}

/**
 * Insert at the beginning of main.c
 */
std::string FileIOHelper::GetIOCode() {
  return GetGlobalFilePointers() + GetIntCode();
}

/**
 * input at the beginning of main
 */
std::string FileIOHelper::GetFilePointersInit() {
  return R"prefix(
  int_input_fp = fopen("/home/hebi/.helium.d/input_values/int.txt", "r");
  char_input_fp = fopen("/home/hebi/.helium.d/input_values/char.txt", "r");
  bool_input_fp = fopen("/home/hebi/.helium.d/input_values/bool.txt", "r");
  // this file will not only record the output, but also the input
  output_fp = fopen("helium_output.txt", "w");
)prefix";
}

/**
 * Reset the mapping.
 * If not, the result is serious: the helper functions are accumulating across POIs!
 */
void IOHelper::Reset() {
  /**
   * Primitive types
   */
  m_input["int"] = R"prefix(
void input_int(int *var) {
  scanf("%d", var);
}
)prefix";

  m_output["int"] = R"prefix(
void output_int(int *var, const char *name) {
  printf("int_%s=%d\n", name, *var); fflush(stdout);
}
)prefix";

  m_input["bool"] = R"prefix(
void input_bool(bool *var) {
  int size;
  scanf("%d", &size);
  if (size==0) (*var)=false;
  else (*var)=true;
}
)prefix";

  m_output["bool"] = R"prefix(
void output_bool(bool *var, const char *name) {
  printf("bool_%s=%d\n", name, (*var)?1:0); fflush(stdout);
}
)prefix";

  m_input["char"] = R"prefix(
void input_char(char *var) {
  scanf("%c", var);
}
)prefix";

  m_output["char"] = R"prefix(
void output_char(char *var, const char *name) {
  printf("char_%s=%c\n", name, *var); fflush(stdout);
}
)prefix";

  m_input["char_star"] = R"prefix(
void input_char_star(char **var) {
  int size;
  scanf("%d",&size);
  if (size==0) {
    (*var)=NULL;
  } else if (size!=0) {
    (*var)=(char*)malloc(sizeof(char)*size);
    hhaddr[hhtop]=(*var);
    hhsize[hhtop]=size;
    hhtop++;
    scanf("%s", (*var));
  }
}
)prefix";

  m_output["char_star"] = R"prefix(
void output_char_star(char **var, const char *name) {
  printf("addr_%s=%p\n", name, (void*)(*var)); fflush(stdout);
  if ((*var) != NULL) {
    printf("int_%s.strlen=%ld\n", name, strlen(*var)); fflush(stdout);
    printf("addr_%s=%p\n", name, (void*)(*var)); fflush(stdout);
    int size = -1;
    for (int i=0;i<hhtop;i++) {
      if (*var == hhaddr[i]) {
        size = hhsize[i]; break;
      }
    }
    if (size != -1) {
      printf("int_%s_heapsize=%d\n", name, size); fflush(stdout);
    }
  }
}
)prefix";


  m_input["char_LJ"] = R"prefix(
void input_char_LJ(char **var) {
  scanf("%s", (*var));
}
)prefix";
  
  m_output["char_LJ"] = R"prefix(
void output_char_LJ(char **var, const char *name) {
  printf("int_%s.size=%d\n", name, sizeof(*var)); fflush(stdout);
  printf("addr_%s=%p\n", name, (void*)(*var)); fflush(stdout);
  printf("int_%s.strlen=%d\n", name, strlen(*var)); fflush(stdout);
}
)prefix";
}

IOHelper::IOHelper() {
  Reset();
}

std::string IOHelper::GetAll() {
  // first, the declaration
  std::string decl;
  std::string code;
  for (auto m : m_input) {
    std::string func = m.second;
    utils::trim(func);
    decl += func.substr(0, func.find('{')-1) + ";\n";
    code += func + "\n";
  }
  // then, the functions
  for (auto m : m_output) {
    std::string func = m.second;
    utils::trim(func);
    decl += func.substr(0, func.find('{')-1) + ";\n";
    code += func + "\n";
  }
  return decl + code;
}
