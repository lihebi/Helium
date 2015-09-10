#ifndef __ENUM_SNIPPET_HPP__
#define __ENUM_SNIPPET_HPP__

#include "snippet/Snippet.hpp"

class EnumSnippet : public Snippet {
public:
  EnumSnippet();
  virtual ~EnumSnippet();
  virtual void GetCode();
  virtual void GetName();
  virtual void GetDependence();
private:
};

#endif
