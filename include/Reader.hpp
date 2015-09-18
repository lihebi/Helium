#ifndef __READER_HPP__
#define __READER_HPP__

#include "Variable.hpp"
#include "segment/Segment.hpp"
#include "Config.hpp"
#include "segment/SegmentProcessUnit.hpp"

#include <pugixml/pugixml.hpp>
#include <vector>

class Reader {
public:
  Reader(const std::string &filename);
  virtual ~Reader();
  void Read();
private:
  void getSegments();
  void getLoopSegments();
  void getAnnotationSegments();

  std::vector<std::shared_ptr<SegmentProcessUnit> > m_seg_units;
  std::shared_ptr<pugi::xml_document> m_doc;
  std::string m_filename;
};


#endif
