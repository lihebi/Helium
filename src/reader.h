#include <pugixml.hpp>
#include <vector>
#include <memory>

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


  std::vector<std::shared_ptr<SegmentProcessUnit> > m_seg_units;
  std::shared_ptr<pugi::xml_document> m_doc;
  std::string m_filename;
  static int m_skip_segment; // store config number
  static int m_cur_seg_no; // statically +1 to record current segment
};
