#ifndef __FUNCTION_SNIPPET_HPP__
#define __FUNCTION_SNIPPET_HPP__

#include "snippet/Snippet.hpp"

class FunctionSnippet : public Snippet {
public:
  FunctionSnippet(const std::string& code) {}
  virtual ~FunctionSnippet() {}
private:
};

#endif
