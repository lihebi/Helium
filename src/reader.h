#include <vector>
#include <memory>

#include "segment.h"
#include "ast.h"
#include "utils.h"

class Reader {
public:
  Reader(const std::string &filename);
  Reader(const std::string &filename, std::vector<int> line_numbers);
  virtual ~Reader() {}
  void Read();
  
  /* meta data for test */
  int GetSegmentCount() {
    return m_segments.size();
  }
  void PrintSegments();
  /**
   * get loc of all segments
   */
  int GetSegmentLOC() {
    int result = 0;
    for (Segment &seg : m_segments) {
      std::string code = seg.GetText();
      result += std::count(code.begin(), code.end(), '\n');
    }
    return result;
  }
private:
  void getLoopSegments();
  void getAnnotationSegments();
  void getDivideSegments();
  void getDivideRecursive(ast::NodeList nodes);

  SegmentList m_segments;
  ast::Doc m_doc;
  std::string m_filename;
  static int m_skip_segment; // store config number
  static int m_cur_seg_no; // statically +1 to record current segment
};
