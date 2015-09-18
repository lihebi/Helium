#ifndef __VARIABLE_SNIPPET_HPP__
#define __VARIABLE_SNIPPET_HPP__

#include "snippet/Snippet.hpp"

class VariableSnippet : public Snippet {
public:
  VariableSnippet(const std::string& code) {}
  virtual ~VariableSnippet() {}
private:
};

#endif
