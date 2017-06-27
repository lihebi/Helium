#ifndef CORNER_H
#define CORNER_H


#include "helium/utils/common.h"

/**
 * Class for manage corner cases
 */
class Corner {
public:
  static Corner *Instance() {
    if (!m_instance) {
      m_instance = new Corner();
    }
    return m_instance;
  }

  std::vector<int> GetIntCorner() {return m_int_corner;}
  std::vector<char> GetCharCorner() {return m_char_corner;}
  std::vector<int> GetStrlenCorner() {return m_strlen_corner;}
private:
  Corner();
  ~Corner();

  void prepIntCorner();
  void prepCharCorner();
  void prepStrlenCorner();
  static Corner *m_instance;
  
  std::vector<int> m_int_corner;
  std::vector<char> m_char_corner;
  std::vector<int> m_strlen_corner;
};

#endif /* CORNER_H */
