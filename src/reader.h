#include <vector>
#include <memory>

#include "segment.h"
#include "ast.h"

class Reader {
public:
  Reader(const std::string &filename);
  virtual ~Reader();
  void Read();
private:
  void getSegments();
  void getLoopSegments();
  void getAnnotationSegments();
  void getDivideSegments();
  void getDivideRecursive(ast::NodeList nodes);


  std::vector<SPU> m_spus;
  ast::Doc m_doc;
  std::string m_filename;
  static int m_skip_segment; // store config number
  static int m_cur_seg_no; // statically +1 to record current segment
};
