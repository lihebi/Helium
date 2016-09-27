#include "point_of_interest.h"
#include "helium_options.h"
#include <iostream>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

namespace fs = boost::filesystem;

PointOfInterest *PointOfInterest::m_instance = NULL;

PointOfInterest::PointOfInterest() {
  std::string folder = HeliumOptions::Instance()->GetString("folder");
  std::string poi_file = HeliumOptions::Instance()->GetString("poi-file");

  // read poi file
  fs::path poi_path(poi_file);
  if (!fs::exists(poi_file)) {
    std::cerr << "EE: point of interest file " << poi_file << " does not exists." << "\n";
    exit(1);
  }

  // check if it exists in folder

  // store the POIs associated with the folder
  // the "benchmark" field must matches the folder name!
}


TEST(FileSystemCase, PathTest) {
  fs::path home("~");
  EXPECT_EQ(home.string(), "~");
  fs::path home_can = fs::canonical(home);
  EXPECT_EQ(home_can.string(), "/home/hebi");
}
