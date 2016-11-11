#include "sequential_type.h"
#include "primitive_type.h"
#include "utils/log.h"
#include "type_helper.h"
#include "utils/utils.h"
#include "corner.h"
#include "helium_options.h"
#include "io_helper.h"
#include <iostream>

/**
 * char aaa[5]
 * ArrayType(char, 5)
 */
ArrayType::ArrayType(std::string type_str, int num) {
  m_num = num;
  m_contained_type = TypeFactory::CreateType(type_str);
}

ArrayType::~ArrayType() {
  if (m_contained_type) {
    delete m_contained_type;
  }
}

void ArrayType::GenerateIOFunc() {
  // TODO This does not support multi-dimension array
  std::string raw = GetRaw();
  char buf[BUFSIZ];
  std::string key = IOHelper::ConvertTypeStr(raw);
  std::string contain = m_contained_type->GetRaw();
  std::string contain_key = IOHelper::ConvertTypeStr(contain);
  if (IOHelper::Instance()->Has(key)) {
    return;
  }
  const char *input_format = R"prefix(
void input_%s(%s **var) { // key, contain
  int size;
  scanf("%s", &size); // replace to d
  if (szie==0) {
    (*var)=NULL;
  } else {
    for (int i=0;i<size;i++) {
      input_%s(&(*var)[i]); // contain_key
    }
  }
}
)prefix";
  sprintf(buf, input_format,
          key.c_str(), contain.c_str(),
          contain_key.c_str());
  std::string input = buf;

  const char *output_format = R"prefix(
void output_%s(%s *var, const char *name) { // key, contain
  printf("int_%s.size=%s\n", name, sizeof(var)); fflush(stdout); // replace with s and d
  printf("addr_%s=%s\n", name, (void*)var); fflush(stdout); // replace with s and p
  for (int i=0;i<%d;i++) { // replace with m_num
    output_%s((*var)[i]); // replace with contain_key
  }
}
)prefix";
  sprintf(buf, output_format,
          key.c_str(), contain.c_str(),
          "%s", "%d",
          "%s", "%p",
          m_num,
          contain_key.c_str());
  std::string output=buf;
  IOHelper::Instance()->Add(key, input, output);
}

std::string ArrayType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// ArrayType::GetDeclCode\n";
  if (!m_contained_type) {
    ret += "// [WW] ArrayType::GetDeclCode with no contained type\n";
    return ret;
  }
  if (m_num == 0) {
    ret += "// [WW] ArrayType::GetDeclCode array size is 0\n";
    return ret;
  }
  ret += m_contained_type->GetDeclCode(var + "[" + std::to_string(m_num) + "]");
  return ret;
}

/**
 * If contained type is "char", use scanf("%s)
 * Otherwise, init the first index if available.
 */
std::string ArrayType::GetInputCode(std::string var, bool simple) {
  std::string ret;
  ret += "// ArrayType::GetInputCode\n";
  if (!m_contained_type) {
    ret += "// ArrayType::GetInputCode with no contained type\n";
    return ret;
  }
  if (m_num == 0) {
    ret += "ArrayType::GetInputCode array size is 0";
    return ret;
  }
  std::string raw = GetRaw();
  std::string key = IOHelper::ConvertTypeStr(raw);
  

  if (dynamic_cast<CharType*>(m_contained_type)) {
    ret += get_scanf_code("%s", var);
  } else {
    for (int i=0;i<m_num;i++) {
      ret += m_contained_type->GetInputCode(var + "[" + std::to_string(i) + "]", simple);
    }
  }
  
  return ret;
}

std::string ArrayType::GetOutputCode(std::string var, bool simple) {
  std::string ret;
  if (!m_contained_type) {
    helium_log_warning("ArrayType::GetOutputCode with no contained type");
    return "";
  }
  if (m_num == 0) {
    helium_log_warning("ArrayType::GetOutputCode array size is 0");
    return "";
  }
  ret += get_sizeof_printf_code(var);
  ret += get_addr_printf_code(var);
  if (dynamic_cast<CharType*>(m_contained_type)) {
    ret += get_strlen_printf_code(var);
  } else {
    for (int i=0;i<m_num;i++) {
      ret += m_contained_type->GetOutputCode(var + "[" + std::to_string(i) + "]", simple);
    }
  }
  return ret;
}

InputSpec *ArrayType::GenerateRandomInput(bool simple) {
  helium_print_trace("ArrayType::GenerateRandomInput");
  InputSpec *spec = NULL;
  if (!m_contained_type) {
    helium_log_warning("ArrayType::GetOutputCode with no contained type");
    return NULL;
  }
  if (m_num == 0) {
    helium_log_warning("ArrayType::GetOutputCode array size is 0");
    return NULL;
  }
  if (dynamic_cast<CharType*>(m_contained_type)) {
    int len = utils::rand_int(0, m_num);
    std::string str = utils::rand_str(len);
    spec = new InputSpec("", str);
  } else {
    spec = new ArrayInputSpec();
    for (int i=0;i<m_num;i++) {
      InputSpec *tmp_spec = m_contained_type->GenerateRandomInput(simple);
      spec->Add(tmp_spec);
    }
  }
  return spec;
}


/********************************
 * PointerType
 *******************************/

PointerType::PointerType(std::string type_str) {
  m_contained_type = TypeFactory::CreateType(type_str);
}

PointerType::~PointerType() {
  if (m_contained_type) {
    delete m_contained_type;
  }
}

void PointerType::GenerateIOFunc() {
  std::string contain = m_contained_type->GetRaw();
  std::string key=IOHelper::ConvertTypeStr(contain+"*");
  std::string contain_key = IOHelper::ConvertTypeStr(contain);
  if (IOHelper::Instance()->Has(key)) {
    return;
  }
  std::string format;
  char buf[BUFSIZ];
  std::string input,output;
  /**
   * INPUT
   */
  format = R"prefix(
void input_%s(%s **var) { // key, contain
  int size;
  scanf("%s", &size); // replace to d
  if (size==0) {
    (*var)=NULL;
  } else {
    (*var) = (%s*)malloc(sizeof(%s)*size); // contain, contain
    hhaddr[hhtop]=(*var);
    hhsize[hhtop]=size;
    hhtop++;
    for (int i=0;i<size;i++) {
      input_%s(&(*var)[i]); // contain_key
    }
  }
}
)prefix";
  sprintf(buf, format.c_str(),
          key.c_str(), contain.c_str(), "%d",
          contain.c_str(), contain.c_str(), contain_key.c_str());
  input=buf;
  /**
   * Output
   */
  format = R"prefix(
void output_%s(%s *var, const char *name) { // key, contain
  if (var == NULL) {
    printf("isnull_%s = %s\n", name, 1); fflush(stdout); // replace with s and d
  } else {
    printf("isnull_%s = %s\n", name, 0); fflush(stdout); // replace with s and d
    int size=-1;
    for (int i=0;i<hhtop;i++) {
      if (var == hhaddr[i]) {
        size = hhsize[i]; break;
      }
    }
    if (size != -1) {
      printf("int_%s_heapsize = %s\n", name, size); fflush(stdout); // replace with s and d
      for (int i=0;i<size;i++) {
        char hbuf[BUFSIZ]; // may be a global variable
        sprintf(hbuf, "%s[%s]", name, i); // replace with s and d
        output_%s(var[i], hbuf); // contain_key
      }
    }
  }
}
)prefix";
  sprintf(buf, format.c_str(),
          key.c_str(), contain.c_str(),
          "%s", "%d",
          "%s", "%d",
          "%s", "%d",
          "%s", "%d",
          contain_key.c_str());
  output=buf;
  // Adding
  IOHelper::Instance()->Add(key, input, output);
  // Contained Type
  m_contained_type->GenerateIOFunc();
}

std::string PointerType::GetDeclCode(std::string var) {
  if (!m_contained_type) {
    helium_log_warning("PointerType::GetDeclCode with no contained type");
    return "";
  }
  std::string ret;
  ret += "// PointerType::GetDeclCode: " + var + "\n";
  if (m_raw.empty()) {
    ret += m_contained_type->GetDeclCode("*" + var);
  } else {
    ret += m_raw + " var;\n";
  }
  return ret;
}

std::string PointerType::GetInputCode(std::string var, bool simple) {
  if (!m_contained_type) {
    helium_log_warning("PointerType::GetInputCode with no contained type");
    return "";
  }
  // std::string ret;
  // ret += "// PointerType::GetInputCode: " + var + (simple?"simple":"") +"\n";
  std::string contain = m_contained_type->GetRaw();
  std::string key = IOHelper::ConvertTypeStr(contain+"*");
  std::string func = "input_"+key;
  std::string call = func + "(&"+var+");\n";
  GenerateIOFunc();
  return call;
}

std::string PointerType::GetOutputCode(std::string var, bool simple) {
  std::string contain = m_contained_type->GetRaw();
  std::string key = IOHelper::ConvertTypeStr(contain+"*");
  GenerateIOFunc();
  return IOHelper::GetOutputCall(key, var, "\""+var+"\"");
}

InputSpec *PointerType::GenerateRandomInput(bool simple) {
  // std::cout << ToString() << "\n";
  InputSpec *ret = NULL;
  helium_print_trace("PointerType::GenerateRandomInput");
  if (!m_contained_type) {
    helium_log_warning("PointerType::GenerateRandomInput with no contained type");
    std::cerr << "EE: PointerType::GenerateRandomInput with no contained type" << "\n";
    return NULL;
  }

  if (dynamic_cast<CharType*>(m_contained_type)) {
    int max_strlen = HeliumOptions::Instance()->GetInt("test-input-max-strlen");
    int helium_size = utils::rand_int(0, max_strlen+1);
    std::string str;
    if (helium_size == 0) {
      str="";
    } else {
      int len = utils::rand_int(0, helium_size);
      str = utils::rand_str(len);
    }
    std::string raw = std::to_string(helium_size) + " " + str;
    std::string spec = "{strlen: " + std::to_string(str.length()) + ", size: " + std::to_string(helium_size) + "}";
    ret = new InputSpec(spec, raw);
  } else {
    ret = new PointerInputSpec();
    int max_pointer_size = HeliumOptions::Instance()->GetInt("test-input-max-pointer-size");
    int helium_size = utils::rand_int(0, max_pointer_size+1);

    // I'm going to add boundary values here
    // This doesnt need to use pairwise, because pairwise is talking about interaction between different variables
    // What exactly I need is boundary values
    // Since I don't have a separate method to generate boundary test for now,
    // I'm going to hard code the boundary, for pointer type, here.
    // Wait, I might want to limit the option: test-input-max-pointer-size
    // to, may be 2
    
    for (int i=0;i<helium_size;i++) {
      InputSpec *tmp_spec = m_contained_type->GenerateRandomInput(simple);
      ret->Add(tmp_spec);
    }
  }
  helium_print_trace("PointerType::GenerateRandomInput END");
  return ret;
}

std::vector<InputSpec*> PointerType::GeneratePairInput() {
  std::vector<InputSpec*> ret;
  // TODO
  return ret;
}
