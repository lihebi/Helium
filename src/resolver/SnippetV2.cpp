#include "helium/resolver/SnippetV2.h"
#include "helium/utils/fs_utils.h"
using namespace v2;



void Snippet::readCode() {
  int beginLine = Begin.getLine();
  int beginColumn = Begin.getColumn();
  int endLine = End.getLine();
  int endColumn = End.getColumn();
  
  std::ifstream is;
  is.open(File);
  int l=0;
  std::string ret;
  if (is.is_open()) {
    std::string line;
    while(getline(is, line)) {
      l++;
      if (l < beginLine || l > endLine) {
      } else if (l>beginLine && l < endLine) {
        ret += line + "\n";
      } else if (l==beginLine && l == endLine) {
        ret += line.substr(beginColumn-1, endColumn+1 - beginColumn);
      } else if (l== endLine) {
        ret += line.substr(0, endColumn);
      } else if (l==beginLine) {
        ret += line.substr(beginColumn-1) + "\n";
      } else {
        break;
      }
    }
    is.close();
  }
  Code = ret;
}

/**
 * This performs the char-by-char read.
 * The variable does not stop until a semi-colon
 * I know this is a trick.
 */
void VarSnippet::readCode() {
  // this should read until a semi-colon
  std::ifstream is;
  is.open(File);
  std::string ret;
  assert(is.is_open());
  int line=1,col=0;
  char c;
  while (is.get(c)) {
    if (c=='\n') {line++;col=0;}
    else col++;

    SourceLocation loc(line,col);

    if (loc < Begin) ;
    else if (loc >= Begin && loc <= End ) ret+=c;
    else {
      is.unget();
      break;
    }
  }
  while (is.get(c)) {
    if (c==';') break;
    ret += c;
  }
  is.close();
  Code = ret;
}
