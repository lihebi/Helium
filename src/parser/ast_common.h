#ifndef AST_COMMON_H
#define AST_COMMON_H

#include "common.h"

typedef enum {
  CDK_NULL, // do not continue, because this variable is defined on itself. e.g. a = foo(a)
  CDK_This, // this node does defined this variable: a = foo(b,c)
  CDK_Continue // nothing found related to this variable, please continue search previous sibling and parent
} CheckDefKind;

CheckDefKind check_def(std::string code, std::string id);


#endif /* AST_COMMON_H */
