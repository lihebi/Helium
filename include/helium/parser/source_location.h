#ifndef SOURCE_LOCATION_H
#define SOURCE_LOCATION_H

#include <ostream>

/**
 * It is passed by value. So the size is pretty critical.
 * But I'm not going to worry about this so much as I would like.
 */
class SourceLocation {
public:
  SourceLocation(int line, int column)
    : line(line), column(column) {}
  ~SourceLocation() {}

  bool operator<(SourceLocation &rhs) {
    if (this->line < rhs.line) return true;
    else if (this->line == rhs.line && this->column < rhs.column) return true;
    else return false;
  }
  bool operator<=(SourceLocation &rhs) {
    if (this->line < rhs.line) return true;
    else if (this->line == rhs.line && this->column <= rhs.column) return true;
    else return false;
  }
  bool operator>(SourceLocation &rhs) {
    if (this->line > rhs.line) return true;
    else if (this->line == rhs.line && this->column > rhs.column) return true;
    else return false;
  }
  bool operator>=(SourceLocation &rhs) {
    if (this->line > rhs.line) return true;
    else if (this->line == rhs.line && this->column >= rhs.column) return true;
    else return false;
  }
  bool operator==(SourceLocation &rhs) {
    return this->line == rhs.line && this->column == rhs.column;
  }
  friend std::ostream& operator<<(std::ostream &os, const SourceLocation &loc);
  
  int getLine() {return line;}
  int getColumn() {return column;}
private:
  int line = -1;
  int column = -1;
};

std::ostream& operator<<(std::ostream &os, const SourceLocation &loc);

// class SourceRange {
// public:
//   SourceRange() {}
//   ~SourceRange() {}
//   SourceLocation getBegin() {return Begin;}
//   SourceLocation getEnd() {return End;}
// private:
//   SourceLocation Begin;
//   SourceLocation End;
// };

#endif /* SOURCE_LOCATION_H */
