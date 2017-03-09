#include "helium/type/corner.h"

Corner* Corner::m_instance = NULL;

Corner::Corner() {
  prepIntCorner();
  prepCharCorner();
  prepStrlenCorner();
}
Corner::~Corner() {
}

void Corner::prepIntCorner() {
  std::set<int> choices = {-2, -1, 0, 1, 2, 10, 100, 100};
  for (int i=0;i<10;i++) {
    choices.insert(pow(2, i));
  }
  choices.insert(BUFSIZ);
  // TODO literal values from program
  std::set<int> additional;
  for (int a : choices) {
    additional.insert(a-2);
    additional.insert(a-1);
    additional.insert(a+1);
    additional.insert(a+2);
  }
  choices.insert(additional.begin(), additional.end());
  std::vector<int> v(choices.begin(), choices.end());
  m_int_corner = v;
}

void Corner::prepCharCorner() {
  std::vector<char> choices = {'\0', '\t', '\v', '\n'};
  m_char_corner = choices;
}

void Corner::prepStrlenCorner() {
  std::set<int> strlen_choices = {1024, BUFSIZ};
  std::set<int> additional;
  for (int len : strlen_choices) {
    additional.insert(len-2);
    additional.insert(len-1);
    additional.insert(len+1);
    additional.insert(len+2);
  }
  strlen_choices.insert(additional.begin(), additional.end());
  std::vector<int> v(strlen_choices.begin(), strlen_choices.end());
  m_strlen_corner = v;
}

