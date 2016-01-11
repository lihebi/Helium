#include <vector>
#include <memory>

#include "segment.h"
#include "ast.h"
#include "utils.h"

class Reader {
public:
  Reader(const std::string &filename) : m_filename(filename) {
    utils::file2xml(filename, m_doc);
  }
  virtual ~Reader() {}
  void SelectSegments();
  void Read();
  
  /* meta data for test */
  int GetSegmentCount() {
    return m_segments.size();
  }
  
private:
  void getSegments();
  void getLoopSegments();
  void getAnnotationSegments();
  void getDivideSegments();
  void getDivideRecursive(ast::NodeList nodes);


  std::vector<SPU> m_spus;
  SegmentList m_segments;
  ast::Doc m_doc;
  std::string m_filename;
  static int m_skip_segment; // store config number
  static int m_cur_seg_no; // statically +1 to record current segment
};
