#ifndef FAILURE_POINT_H
#define FAILURE_POINT_H

#include "common.h"

class FailurePoint {
public:
  FailurePoint(std::string path, std::string filename, int linum, std::string type)
    : m_path(path), m_filename(filename), m_linum(linum), m_type(type) {}
  ~FailurePoint() {}
  std::string GetPath() {
    return m_path;
  }
  std::string GetFilename() {
    return m_filename;
  }
  std::string GetType() {
    return m_type;
  }
  int GetLinum() {
    return m_linum;
  }
private:
  std::string m_path;
  std::string m_filename;
  int m_linum = 0;
  std::string m_type;
};

class FailurePointFactory {
public:
  /**
   * Create failure point by a single file
   */
  static FailurePoint *CreateFailurePoint(std::string file);
  /**
   * @param file a file contains many failure points for each bug
   */
  static FailurePoint *CreateFailurePoint(std::string file, std::string name);
};

#endif /* FAILURE_POINT_H */
