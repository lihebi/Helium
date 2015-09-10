#ifndef __STRUCTURE_SNIPPET_HPP__
#define __STRUCTURE_SNIPPET_HPP__

#include "snippet/Snippet.hpp"

class StructureSnippet : public Snippet {
public:
  StructureSnippet();
  virtual ~StructureSnippet();
  virtual void GetCode();
  virtual void GetName();
  virtual void GetDependence();
private:
};

#endif
