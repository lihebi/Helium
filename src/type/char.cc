#include "type.h"
#include "type_common.h"
#include "type_helper.h"

using namespace type;

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
    if (Config::Instance()->GetBool("instrument-address")) {
      ret += get_addr_input(var);
    }
    // FIXME this should be less than helium_size? Or just let the oracle do the trick
    // ret += get_scanf_code("%s", var);
  } else if (m_pointer == 2) {
    assert(m_dimension == 0 && "do not support array of pointer for now.");
    ret += get_scanf_code("%d", "&helium_size");
    ret += get_malloc_code(var, "char*", "helium_size");
    if (Config::Instance()->GetBool("instrument-address")) {
      ret += get_addr_input(var);
    }
    ret += get_helium_size_loop("int helium_size;\n" + get_str_input_code(var + "[i]"));
  } else {
    assert(false && "char ***");
  }
  return ret;
}


TestInput* Char::GetTestInputSpec(std::string var) {
  std::string raw;
  std::vector<int> strlens;
  std::vector<int> bufsizs;
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
    bufsizs.push_back(size);
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
      bufsizs.push_back(size2);
      strlens.push_back(len);
    }
  } else {
    assert(false);
  }
  CharTestInput *ret = new CharTestInput(this, var);
  ret->SetRaw(raw);
  ret->SetStrlen(strlens);
  ret->SetBufSize(bufsizs);
  return ret;
}

std::string Char::getOutputCode_Zero(std::string var) {
  std::string ret;
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
  return ret;
}

std::string Char::getOutputCode_One(std::string var) {
  std::string ret;
  assert(m_dimension == 0 && "do not support array of pointer for now.");
  // FIXME For a char pointer, do not output the size.
  // FIXME why do not output?
  // ret += get_sizeof_output(var);
  if (Config::Instance()->GetBool("instrument-strlen")) {
    ret += get_strlen_output(var);
  }
  if (Config::Instance()->GetBool("instrument-address")) {
    ret += get_addr_output(var);
  }
  if (Config::Instance()->GetBool("instrument-null")) {
    ret += get_check_null(var,
                          get_null_output(var, true),
                          get_null_output(var, false)
                          );
  }
  return ret;
}

std::string Char::getOutputCode_Two(std::string var) {
  std::string ret;
  assert(m_dimension == 0 && "do not support array of pointer for now.");
  // TODO the size of the buffer?
  ret += get_check_null(var,
                        (Config::Instance()->GetBool("instrument-null") ? get_null_output(var, false) : "")
                        ,
                        (Config::Instance()->GetBool("instrument-null") ? get_null_output(var, true) : "")
                        + (Config::Instance()->GetBool("instrument-address") ? get_addr_output(var) : "")
                        + get_check_null("*"+var,
                                         (Config::Instance()->GetBool("instrument-null") ? get_null_output("*"+var, false) : "")
                                         ,
                                         (Config::Instance()->GetBool("instrument-null") ? get_null_output("*"+var, true) : "")
                                         + (Config::Instance()->GetBool("instrument-strlen") ? get_strlen_output("*"+var) : "")
                                         + (Config::Instance()->GetBool("instrument-address") ? get_addr_output("*"+var) : "")
                                         )
                        );
  return ret;
}


/**
 * TODO dimension and pointer
 */
std::string Char::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// Char::GetOutputCode " + var + " Dimension: " + std::to_string(m_pointer) + "\n";
  if (m_pointer == 0) {
    ret += getOutputCode_Zero(var);
  } else if (m_pointer == 1) {
    ret += getOutputCode_One(var);
  } else if (m_pointer == 2) {
    ret += getOutputCode_Two(var);
  }
  return ret;
}



std::string CharTestInput::dump() {
  std::string ret;
  ret += m_type->ToString() + " " + m_var + "\n";
  // ret += "size: " + std::to_string(m_strlens.size()) + ", ";
  // ret += "strlens:";
  // for (int len : m_strlens) {
  //   ret += " " + std::to_string(len);
  // }
  // ret += ", bufsizs:";
  // for (int size : m_bufsizs) {
  //   ret += " " + std::to_string(size);
  // }
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
    ret += "Id_bufsiz(" + m_var + "[" + std::to_string(i) + "]) = " + std::to_string(m_bufsizs[i]) + "\n";
  }
  return ret;
}
