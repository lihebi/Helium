#ifndef __READER_HPP__
#define __READER_HPP__

#include <IOVariable.hpp>
#include <Segment.hpp>
#include <Support.hpp>
#include <Config.hpp>
#include <vector>
#include "SegmentUnit.hpp"

#include <pugixml/pugixml.hpp>


class Reader {
public:
  Reader(const std::string &filename, const Config &config);
  virtual ~Reader();
private:
  void getSegments();
  void getLoopSegments();
  void getAnnotationSegments();

  std::vector<SegmentUnit> m_seg_units;
  Config m_config;
  std::shared_ptr<pugi::xml_document> m_doc;
  std::string m_filename;
};


#endif
