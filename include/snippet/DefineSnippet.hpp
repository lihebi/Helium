#ifndef __DEFINE_SNIPPET_HPP__
#define __DEFINE_SNIPPET_HPP__

#include "snippet/Snippet.hpp"

class DefineSnippet : public Snippet {
public:
  DefineSnippet();
  virtual ~DefineSnippet();
  virtual void GetCode();
  virtual void GetName();
  virtual void GetDependence();
private:
};

#endif
