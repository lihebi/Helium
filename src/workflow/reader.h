#include <vector>
#include <memory>

#include "workflow/segment.h"
#include "parser/ast.h"
#include "utils/utils.h"

class POISpec {
public:
  std::string filename;
  int linum = 0;
  std::string type;
};

class Reader {
public:
  Reader(const std::string &filename);
  Reader(std::string filename, POISpec poi);
  // Reader(const std::string &filename, std::vector<int> line_numbers);
  virtual ~Reader() {}
  void Read();
  
  /* meta data for test */
  // int GetSegmentCount() {
  //   return m_segments.size();
  // }
  void PrintSegments();
  /**
   * get loc of all segments
   */
  // int GetSegmentLOC() {
  //   int result = 0;
  //   for (Segment &seg : m_segments) {
  //     std::string code = seg.GetText();
  //     result += std::count(code.begin(), code.end(), '\n');
  //   }
  //   return result;
  // }
  static void slice(std::string slice_file, std::string benchmark_folder);
private:
  // void getLoopSegments();
  void getAnnotationSegments();
  Segment* getAnnotSeg();
  Segment* getAnnotLoop();
  // void getDivideSegments();
  // void getDivideRecursive(ast::NodeList nodes);
  void getFuncCallSegments(std::string func_name);
  void GA();
  // SegmentList m_segments;
  ast::Doc *m_doc;
  std::string m_filename;
  static int m_skip_segment; // store config number
  static int m_cur_seg_no; // statically +1 to record current segment
};
