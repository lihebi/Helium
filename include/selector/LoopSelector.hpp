#ifndef __LOOP_SELECTOR_HPP__
#define __LOOP_SELECTOR_HPP__

#include "selector/Selector.hpp"

class LoopSelector : public Selector {
public:
  LoopSelector();
  virtual ~LoopSelector();
  virtual void Select();
private:
};

#endif
