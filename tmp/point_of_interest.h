#ifndef POINT_OF_INTEREST_H
#define POINT_OF_INTEREST_H
#include <string>
#include <vector>
/**
 * The format of POI file should be a csv file.
 * It must contains a header, with the specified name of columns.
 * benchmark, file, linum, type
 */
class PointOfInterest {
public:
  PointOfInterest(std::string folder, std::string file, int linum, std::string type, std::string failure_condition)
    : m_folder(folder), m_file(file), m_linum(linum), m_type(type), m_failure_condition(failure_condition) {}
  ~PointOfInterest() {}

  std::string GetFilename() {return m_file;}
  std::string GetPath();

  int GetLinum() {return m_linum;}
  std::string GetType() {return m_type;}
  std::string GetFailureCondition() {return m_failure_condition;}
private:
  std::string m_folder;
  std::string m_file;
  int m_linum;
  std::string m_type;

  std::string m_failure_condition;
};

class POIFactory {
public:
  static std::vector<PointOfInterest*> Create(std::string folder, std::string poi_file);
private:
};

// std::vector<PointOfInterest*> create_point_of_interest(std::string folder, std::string poi_file);

#endif /* POINT_OF_INTEREST_H */
