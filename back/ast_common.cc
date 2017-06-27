#include "helium/parser/ast_common.h"
#include "helium/utils/utils.h"

/**
 * This only check the first "="
 */
CheckDefKind check_def(std::string code, std::string id) {
  // remove double ==
  for (size_t pos=code.find("==");
       pos!=std::string::npos;
       pos=code.find("==")) {
    code.erase(pos, 2);
  }
  // remove !=
  for (size_t pos=code.find("!=");
       pos!=std::string::npos;
       pos=code.find("!=")) {
    code.erase(pos, 2);
  }
  // find =, and treat left and right
  if (code.find('=') != std::string::npos) {
    std::string left = code.substr(0, code.find_first_of('='));
    std::string right = code.substr(code.find_first_of('='));
    // FIXME but the left may be *var
    boost::regex reg("\\b" + id + "\\b");
    utils::trim(left);
    if (regex_search(left, reg) && left[0] != '*') {
      if (regex_search(right, reg)) {
        return CDK_NULL;
      } else {
        return CDK_This;
      }
    }
  }
  return CDK_Continue;
}
