#ifndef __VARIABLE_SNIPPET_HPP__
#define __VARIABLE_SNIPPET_HPP__

#include "snippet/Snippet.hpp"

class VariableSnippet : public Snippet {
public:
  VariableSnippet();
  virtual ~VariableSnippet();
  virtual void GetCode();
  virtual void GetName();
  virtual void GetDependence();
private:
};

#endif
