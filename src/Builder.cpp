#include <Builder.hpp>

Builder::Builder(const SegmentUnit &seg_unit, const Config &config)
: m_config(config) {
// instrument input
// instrument output
// resolve to get support
// output code and compile
// get executable name
}
Builder::~Builder() {}

bool Builder::Success() {
  return true;
}
std::string Builder::GetExecutable() {
  return m_executable;
}
