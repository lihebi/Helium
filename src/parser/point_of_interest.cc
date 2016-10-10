#include "point_of_interest.h"
#include "helium_options.h"
#include <iostream>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include "utils/csv.h"
#include "utils/log.h"
#include "utils/fs_utils.h"

namespace fs = boost::filesystem;

PointOfInterest::PointOfInterest(std::string folder, std::string file, int linum, std::string type)
  : m_folder(folder), m_file(file), m_linum(linum), m_type(type) {
}

std::string PointOfInterest::GetFilename() {
  return m_file;
}

std::string PointOfInterest::GetPath() {
  fs::path p = fs::path(m_folder) / "cpped" / m_file;
  return p.string();
}

/**
 * @param folder the folder of benchmark. This folder must contains some sub-folders: cpped, snippets, src. This folder name is the benchmark name.
 * @param poi_file a csv file
 */
std::vector<PointOfInterest*> create_point_of_interest(std::string folder, std::string poi_file) {
  helium_print_trace("create_point_of_interest");
  folder = utils::escape_tide(folder);
  poi_file = utils::escape_tide(poi_file);
  // read poi file
  fs::path poi_path(poi_file);
  if (!fs::exists(poi_file)) {
    std::cerr << "EE: point of interest file " << poi_file << " does not exists." << "\n";
    exit(1);
  }

  // check if it exists in folder
  CSV *csv = CSVFactory::CreateCSV(poi_file);
  if (!csv) {
    std::cerr << "EE: point of interest file " << poi_file << " is not valid." << "\n";
    exit(1);
  }
  // benchmark, file, linum, type
  std::vector<std::string> benchmark, file, linum, type;
  benchmark = csv->Column("benchmark");
  file = csv->Column("file");
  linum = csv->Column("linum");
  type = csv->Column("type");
  
  if (benchmark.empty() || file.empty() || linum.empty() || type.empty()) {
    std::cerr << "EE: point of interest file " << poi_file << " is not valid."
              << "All fields (benchmark, file, linum, type) should be available."
              << "\n";
    exit(1);
  }

  // get the benchmark name
  fs::path target = fs::canonical(folder);
  std::string benchmark_name = target.filename().string();

  // store the POIs associated with the folder
  std::vector<PointOfInterest*> ret;
  for (int i=0;i<(int)benchmark.size();i++) {
    // the "benchmark" field must matches the folder name!
    if (benchmark[i] == benchmark_name) {
      // ret.push_back(new PointOfInterest({benchmark[i], file[i], stoi(linum[i]), type[i]}));
      ret.push_back(new PointOfInterest({folder, file[i], stoi(linum[i]), type[i]}));
    }
  }
  return ret;
}

