#include "type.h"
#include "utils/log.h"
#include "type_helper.h"
#include "utils/utils.h"
#include "config/config.h"

ArrayType::ArrayType() {}

ArrayType::~ArrayType() {}

std::string ArrayType::GetDeclCode(std::string var) {
  std::string ret;
  if (m_contained_type) {
    if (m_num == -1) {
      helium_log_warning("ArrayType::GetDeclCode array size is not available");
    } else {
      ret += m_contained_type->GetDeclCode(var + "[" + std::to_string(m_num) + "]");
    }
  } else {
    helium_log_warning("ArrayType::GetDeclCode with no contained type");
  }
  return ret;
}

/**
 * If contained type is "char", use scanf("%s)
 * Otherwise, init the first index if available.
 */
std::string ArrayType::GetInputCode(std::string var) {
  std::string ret;
  if (m_contained_type) {
    if (m_num > 0) {
      // check if the contained type is "char"
      if (dynamic_cast<CharType*>(m_contained_type)) {
        // FIXME make sure no overflow
        ret += get_scanf_code("%s", var);
      } else {
        ret += m_contained_type->GetInputCode(var + "[0]");
      }
    } else {
      helium_log_warning("ArrayType::GetInputCode array size is not available");
    }
  } else {
    helium_log_warning("ArrayType::GetInputCode with no contained type");
  }
  return ret;
}

std::string ArrayType::GetOutputCode(std::string var) {
  std::string ret;
  if (m_contained_type) {
    if (m_num > 0) {
      // ret += m_contained_type->GetOutputCode(var + "[0]");
      if (dynamic_cast<CharType*>(m_contained_type)) {
        // TODO configurable
        ret += get_strlen_output(var);
        ret += get_sizeof_output(var);
      } else {
        ret += m_contained_type->GetOutputCode(var + "[0]");
      }
    } else {
      helium_log_warning("ArrayType::GetOutputCode array size is not available");
    }
  } else {
    helium_log_warning("ArrayType::GetOutputCode with no contained type");
  }
  return ret;
}

InputSpec *ArrayType::GenerateInput() {
  InputSpec *spec = NULL;
  spec = new InputSpec();
  // then, according to size of array, call for contained type
  if (m_contained_type) {
    if (m_num > 0) {
      // generate no more than m_num value
      if (dynamic_cast<CharType*>(m_contained_type)) {
        // generate a string
      } else {
        // fill only the first one
        InputSpec *tmp_spec = m_contained_type->GenerateInput();
        spec->Add(tmp_spec);
        delete tmp_spec;
      }
    }
  } else {
    helium_log_warning("ArrayType::GenerateInput with no contained type");
  }
  return spec;
}






PointerType::PointerType() {}

PointerType::~PointerType() {}

std::string PointerType::GetDeclCode(std::string var) {
  std::string ret;
  if (m_contained_type) {
    // TODO int *a[5] vs int (*a)[5]
    ret += m_contained_type->GetDeclCode("*" + var);
  } else {
    helium_log_warning("PointerType::GetDeclCode with no contained type");
  }
  return ret;
}

/**
 * int *a;
 *
 scanf(helium_size);
 if (helium_size == 0) {
   a  =NULL;
 } else {
   a = malloc();
   INT_GET_INPUT(a[0]);
 }
 *
 * char *a;
 *
 scanf(helium_size);
 if (helium_size == 0) {
   a = NULL;
 } else {
   a = malloc();
   get_scanf(a); FIXME scanf overflow?
 }
 */
std::string PointerType::GetInputCode(std::string var) {
  std::string ret;
  if (m_contained_type) {
    if (dynamic_cast<CharType*>(m_contained_type)) {
      // TODO this should be 0 or 1, indicating NULL or not NULL
      ret += get_scanf_code("%d", "&helium_size");
      ret += get_helium_size_branch(
                                    var + " = NULL;\n",
                                    get_malloc_code(var, m_contained_type->GetValue(), "helium_size")
                                    + get_scanf_code("%s", var)
                                    );
    } else {
      // malloc
      ret += get_scanf_code("%d", "&helium_size");
      ret += get_helium_size_branch(
                                    // false branch, helium_size == 0
                                    var + " = NULL;\n",
                                    // get_malloc_code(var, m_raw_without_pointer, "1")
                                    get_malloc_code(var, m_contained_type->GetValue(), "helium_size")
                                    + m_contained_type->GetInputCode(var + "[0]")
                                    );
    }
  } else {
    helium_log_warning("PointerType::GetInputCode with no contained type");
  }
  return ret;
}

std::string PointerType::GetOutputCode(std::string var) {
  std::string ret;
  if (m_contained_type) {
    // check if "char"
    if (dynamic_cast<CharType*>(m_contained_type)) {
      ret += get_strlen_output(var);
    } else {
      ret += get_check_null(var,
                            get_null_output(var, true),
                            get_null_output(var, false)
                            + m_contained_type->GetOutputCode("*" + var)
                            );
    }
  } else {
    helium_log_warning("PointerType::GetOutputCode with no contained type");
  }
  return ret;
}

InputSpec *PointerType::GenerateInput() {
  InputSpec *ret = NULL;
  ret = new InputSpec();
  std::string raw;
  std::string spec;
  if (m_contained_type) {
    // generate size
    // according to size, generate content
    if (dynamic_cast<CharType*>(m_contained_type)) {
      // FIXME the config "max-strlen" and variable "size" do not match
      int size = utils::rand_int(0, Config::Instance()->GetInt("max-strlen"));
      raw += std::to_string(size);
      spec += "size: " + std::to_string(size);
      int len = utils::rand_int(0, size);
      raw += utils::rand_str(len);
      spec += "strlen: " + std::to_string(len);
      ret->SetRaw(raw);
      ret->SetSpec(spec);
    } else {
      // generate size
      // init first one
      // FIXME configurable
      int size = utils::rand_int(0, 5);
      raw += std::to_string(size);
      if (size > 0) {
        InputSpec *tmp_spec = m_contained_type->GenerateInput();
        ret->Add(tmp_spec);
        delete tmp_spec;
      }
      ret->SetRaw(raw);
    }
  } else {
    helium_log_warning("PointerType::GenerateInput with no contained type");
  }
  return ret;
}
