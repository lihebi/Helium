#ifndef SOURCE_LOCATION_H
#define SOURCE_LOCATION_H

/**
 * It is passed by value. So the size is pretty critical.
 * But I'm not going to worry about this so much as I would like.
 */
class SourceLocation {
public:
  SourceLocation(int line, int column)
    : line(line), column(column) {}
  ~SourceLocation() {}
private:
  int line = -1;
  int column = -1;
};

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
