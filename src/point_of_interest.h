#ifndef POINT_OF_INTEREST_H
#define POINT_OF_INTEREST_H

/**
 * The format of POI file should be a csv file.
 * It must contains a header, with the specified name of columns.
 * benchmark, file, linum, type
 */
class PointOfInterest {
public:
  static PointOfInterest *Instance() {
    if (!m_instance) {
      m_instance = new PointOfInterest();
    }
    return m_instance;
  }
private:
  PointOfInterest();
  ~PointOfInterest() {}
  static PointOfInterest *m_instance;
};

#endif /* POINT_OF_INTEREST_H */
