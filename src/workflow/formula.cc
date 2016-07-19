#include "formula.h"
#include "utils/utils.h"


bool Formula::is_constant(std::string str) {
  // number
  if (utils::is_number(str)) return true;
  // char
  if (str.size() == 3 && str[0] == '\'' && str[2] == '\'') return true;
  // string
  if (str.size() >= 2 && str.front() == '"' && str.back() == '"') return true;
  return false;
}



/**
 */
Formula *FormulaFactory::CreateFormula(std::string str) {
  std::string delim;
  if (str.find("!=") != std::string::npos) {
    delim = "!=";
  } else if (str.find(">=") != std::string::npos) {
    delim = ">=";
  } else if (str.find("<=") != std::string::npos) {
    delim = "<=";
  } else if (str.find("=") != std::string::npos) {
    delim = "=";
  } else if (str.find("<") != std::string::npos) {
    delim = "<";
  } else if (str.find(">") != std::string::npos) {
    delim = ">";
  } else {
    return NULL;
  }
  std::string lsh = str.substr(0, str.find(delim));
  std::string rhs = str.substr(str.find(delim)+delim.length());
  // FIXME I don't want to have this restriction
  if (lsh.find_first_of("+-") != std::string::npos) return NULL;
  std::vector<std::string> rhs_split = utils::split(rhs, "+- ");
  if (rhs_split.size() > 2) {
    return NULL;
  }
  if (rhs_split.size() == 1) {
    return new UnaryFormula(lsh, delim, rhs);
  } else {
    return new BiFormula(lsh, delim, rhs);
  }
  // return new Formula(lsh, delim, rhs);
}

/**
 * Fill out the m_rel
 * Fill out the m_op
 * Fill out lhs
 * fill out rhs vector
 *
 * lhs and/or rhs might be multiple operands
 * TODO exchange them if rhs is one address
 * assert lhs to be a single address
 * Preconditions:
 * - lhs only one address
 * - rhs up to two addresses
 * Wait: rhs can be a constant
 */
Formula::Formula(std::string lhs, std::string op, std::string rhs) {
  m_lhs = lhs;
  if (rhs.find("+") != std::string::npos) {
    m_op = OK_Plus;
  } else if (rhs.find("-") != std::string::npos) {
    m_op = OK_Minus;
  }
  if (op == "!=") m_kind = FK_NonEqual;
  else if (op == ">=") m_kind = FK_LargerEqual;
  else if (op == "<=") m_kind = FK_LessEqual;
  else if (op == "=") m_kind = FK_Equal;
  else if (op == "<") m_kind = FK_Less;
  else if (op == ">") m_kind = FK_Larger;
  m_rhs = utils::split(rhs, "+- ");
}

/**
 * Replace the key in map with the value, for the formula's operants
 */
void Formula::Replace(std::map<std::string, std::string> m) {
  if (m.count(m_lhs) == 1) {
    m_lhs = m[m_lhs];
  }
  for (int i=0;i<(int)m_rhs.size();++i) {
    if (m.count(m_rhs[i]) == 1) {
      m_rhs[i] = m[m_rhs[i]];
    }
  }
}


// 3. check whether they are associated with m_header
// it can be associated with a header, or a constant
bool Formula::Valid() {
  assert(!m_lhs.empty());
  if (!is_constant(m_lhs) && m_lhs[0] != 'I' && m_lhs[0] != 'O') return false;
  for (std::string rhs : m_rhs) {
    assert(!rhs.empty());
    if (!is_constant(rhs) && rhs[0] != 'I' && rhs[0] != 'O') return false;
  }
  return true;
}


/********************************
 * Data Validation
 *******************************/

/**
 * Validate by the data whether the formula is true or false
 */
bool UnaryFormula::Validate() {
  assert(m_rhs_data.size() == m_rhs.size());
  for (std::vector<std::string> &row : m_rhs_data) {
    assert(row.size() == m_lhs_data.size());
  }
  for (int i=0;i<(int)m_lhs_data.size();i++) {
    if (!validate(i)) return false;
  }
  return true;
}

/**
 * FK_Larger,
 * FK_Less,
 * FK_LargerEqual,
 * FK_LessEqual
 * FIXME NOW the data type must matter!
 * Pointer address, int, string
 */
bool UnaryFormula::validate(int idx) {
  std::string lhs = m_lhs_data[idx];
  std::string rhs = m_rhs_data[0][idx];
  std::string lhs_header = m_lhs;
  std::string rhs_header = m_rhs[0];
  switch (m_kind) {
  case FK_Equal:
    if (lhs == rhs) {return true;} break;
  case FK_NonEqual:
    if (lhs != rhs) {return true;} break;
  case FK_Larger:
    if (lhs > rhs) {return true;} break;
  case FK_Less:
    if (lhs< rhs) {return true;} break;
  case FK_LargerEqual:
    if (lhs >= rhs) {return true;} break;
  case FK_LessEqual:
    if (lhs <= rhs) {return true;} break;
  }
  return false;
}

bool BiFormula::Validate() {
  assert(m_rhs_data.size() == m_rhs.size());
  for (std::vector<std::string> &row : m_rhs_data) {
    assert(row.size() == m_lhs_data.size());
  }
  for (int i=0;i<(int)m_lhs_data.size();i++) {
    if (!validate(i)) return false;
  }
  return true;
}


/**
 * TODO support OK_Minus and more
 */
bool BiFormula::validate(int idx) {
  std::string lhs = m_lhs_data[idx];
  std::string rhs1 = m_rhs_data[0][idx];
  std::string rhs2 = m_rhs_data[1][idx];
  std::string lhs_header = m_lhs;
  std::string rhs_header1 = m_rhs[0];
  std::string rhs_header2 = m_rhs[1];
  switch (m_kind) {
  case FK_Equal:
    if (m_op == OK_Plus) {
      if (lhs == rhs1 + rhs2) {return true;} break;
    }
  case FK_NonEqual:
    if (m_op == OK_Plus) {
      if (lhs != rhs1 + rhs2) {return true;} break;
    }
  case FK_Larger:
    if (m_op == OK_Plus) {
      if (lhs > rhs1 + rhs2) {return true;} break;
    }
  case FK_Less:
    if (m_op == OK_Plus) {
      if (lhs< rhs1 + rhs2) {return true;} break;
    }
  case FK_LargerEqual:
    if (m_op == OK_Plus) {
      if (lhs >= rhs1 + rhs2) {return true;} break;
    }
  case FK_LessEqual:
    if (m_op == OK_Plus) {
      if (lhs <= rhs1 + rhs2) {return true;} break;
    }
  }
  return false;
}


/**
 * TODO Validate variance:
 * 1. negate operator (= to !=)
 * 2. add constants to either side
 */
bool UnaryFormula::ValidateVariance() {
  return false;
}
bool BiFormula::ValidateVariance() {
  return false;
}

/**
 * TODO Print the formula out!
 */
std::string UnaryFormula::ToString() {
  return "";
}
std::string BiFormula::ToString() {
  return "";
}
