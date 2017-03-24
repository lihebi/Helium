#include "helium/parser/source_location.h"

std::ostream& operator<<(std::ostream &os, const SourceLocation &loc) {
  os << loc.line << "," << loc.column;
  return os;
}
