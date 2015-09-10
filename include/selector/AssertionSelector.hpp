#ifndef __ASSERTION_SELECTOR_HPP__
#define __ASSERTION_SELECTOR_HPP__

#include "selector/Selector.hpp"

class AssertionSelector : public Selector {
public:
  AssertionSelector();
  virtual ~AssertionSelector();
  virtual void Select();
private:
};

#endif
