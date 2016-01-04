#include <vector>
#include <memory>

#include "segment.h"

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


  std::vector<SPU> m_spus;
  std::shared_ptr<pugi::xml_document> m_doc;
  std::string m_filename;
  static int m_skip_segment; // store config number
  static int m_cur_seg_no; // statically +1 to record current segment
};
