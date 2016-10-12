#include "type.h"
#include "utils/utils.h"
#include "utils/log.h"
#include "resolver/snippet_db.h"
#include <iostream>

/**
 * select from the corner cases
 * @param limit return up to this number of inputs. -1 means no limit, return all corner cases
 */
std::vector<InputSpec*> Type::GenerateCornerInputs(int limit) {
  if (limit == -1) return m_corners;
  std::set<int> idxes = utils::rand_ints(0, m_corners.size(), limit);
  std::vector<InputSpec*> ret;
  for (int idx : idxes) {
    ret.push_back(m_corners[idx]);
  }
  return ret;
}



bool type_has_word(std::string type, std::string word) {
  boost::regex reg("\\b" + word + "\\b");
  if (boost::regex_search(type, reg)) {
    return true;
  }
  return false;
}

std::string type_remove_word(std::string type, std::string word) {
  std::string ret;
  boost::regex reg("\\b" + word + "\\b");
  if (boost::regex_search(type, reg)) {
    ret = boost::regex_replace<boost::regex_traits<char>, char> (type, reg, "");
  }
  return ret;
}

/**
 * "char *"
 */
bool type_is_str(std::string str) {
  if (type_has_word(str, "char")
      && std::count(str.begin(), str.end(), '*') == 1
      && str.find('[') == std::string::npos
      ) {
    return true;
  }
  return false;
}

/**
 * char [3]
 */
bool type_is_buf(std::string str) {
  if (type_has_word(str, "char")
      && str.find('*') == std::string::npos
      && std::count(str.begin(), str.end(), '[') == 1
      ) {
    return true;
  }
  return false;
}

/**
 * Check if it is a valid type that Helium will handle.
 */
bool helium_valid_type(std::string str) {
  std::vector<std::string> valid_tokens {
    "char", "int", "bool", "_Bool"
  };
  std::vector<std::string> invalid_tokens {
    "const", "static", "auto", "extern", "volatile"
  };
  // if there's still something left, it must be a local structure, otherwise put into known
  // do not handle for now, put into unknown!
  str = type_remove_word(str, "char");
  str = type_remove_word(str, "int");
  str = type_remove_word(str, "bool");
  str = type_remove_word(str, "_Bool");

  while (str.back() == ']' || str.back() == '*') {
    if (str.back() == ']') {
      str = str.substr(str.find_last_of('['));
    }
    if (str.back() == '*') {
      str.pop_back();
    }
    utils::trim(str);
  }
  if (str.empty()) {
    return true;
  }
  return false;
}


Type *TypeFactory::CreateType(XMLNode decl_node) {
  std::string type = decl_get_type(decl_node);
  std::vector<std::string> dims = decl_get_dimension(decl_node);
  std::string suffix;
  for (std::string dim : dims) {
    suffix += "[" + dim + "]";
  }
  return TypeFactory::CreateType(type + suffix);
}

/**
 * Current Strategy: First check if it is valid.
 * Only construct type when it is, otherwise create an Unknown type, where only decl is added.
 * But, be sure to handle all valid cases!
 */
Type *TypeFactory::CreateType(std::string str) {
  if (str.empty()) return NULL;
  Type *type;

  // if (helium_valid_type(str)) {
  // } else {
  //   return new UnknownType(str);
  // }

  // 4. check if char*
  // if (type_is_str(str)) {
  //   return new StrType();
  // }

  // 3. check if char[]
  // if (type_is_buf(str)) {
  //   std::string numstr = str.substr(str.find('[')+1, str.find(']'));
  //   int num = 0;
  //   try {
  //     num = stoi(numstr);
  //   } catch (std::exception e) {
  //     helium_log_warning("Exception in TypeFactory::CreateType: " + std::string(e.what()));
  //   }
  //   type = new BufType(num);
  //   return type;
  // }

  // Pointer
  if (str.back() == '*') {
    str.pop_back();
    type = new PointerType(str);
    return type;
  }

  // Array
  if (str.back() == ']') {
    str.pop_back();
    std::string numstr = str.substr(str.find_last_of('[')+1);
    str = str.substr(0, str.find_last_of('['));
    if (numstr.empty()) {
      // [], should be the argument, treat as double pointer
      return new PointerType(str);
    } else {
      // magic number, if the array size is unknown, maybe an expression, use 10 as the size
      int num = 10;
      try {
        num = stoi(numstr);
      } catch (std::exception e) {
        // helium_log_warning("Exception in TypeFactory::CreateType: " + std::string(e.what()));
        std::cerr << "Array size not a number: "  << std::string(e.what()) << ". Using magic number 10.\n";
      }
      type = new ArrayType(str, num);
      return type;
    }
  }
  // Simple Type
  if (type_has_word(str, "char")) {
    return new CharType();
  } else if (type_has_word(str, "int")) {
    return new IntType();
  } else if (type_has_word(str, "bool") || type_has_word(str, "_Bool")) {
    return new BoolType();
  }
  return new UnknownType(str);
  return NULL;
}


/**
 * FIXME need smart pointer!
 */
// std::vector<std::pair<InputSpec*, InputSpec*> > pairwise(Type* a, Type *b) {
//   std::vector<std::pair<InputSpec*, InputSpec*> > ret;
//   std::vector<InputSpec*> apair = a->GeneratePairInput();
//   std::vector<InputSpec*> bpair = b->GeneratePairInput();
//   // combine
//   for (InputSpec* ain : apair) {
//     for (InputSpec *bin : bpair) {
//       ret.push_back({ain, bin});
//     }
//   }
//   return ret;
// }
