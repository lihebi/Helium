#include "helium/parser/point_of_interest.h"
#include "helium/utils/helium_options.h"
#include <iostream>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include "helium/utils/table.h"
#include "helium/utils/log.h"
#include "helium/utils/fs_utils.h"

namespace fs = boost::filesystem;

std::string PointOfInterest::GetPath() {
  fs::path p = fs::path(m_folder) / "cpped" / m_file;
  return p.string();
}

/**
 * @param folder the folder of benchmark. This folder must contains some sub-folders: cpped, snippets, src. This folder name is the benchmark name.
 * @param poi_file a csv file
 */
std::vector<PointOfInterest*> POIFactory::Create(std::string folder, std::string poi_file) {
  helium_print_trace("POIFactory::Create");
  folder = utils::escape_tide(folder);
  poi_file = utils::escape_tide(poi_file);
  // read poi file
  fs::path poi_path(poi_file);
  if (!fs::exists(poi_file)) {
    std::cerr << "EE: point of interest file " << poi_file << " does not exists." << "\n";
    exit(1);
  }

  Table *table = TableFactory::Create(poi_file);
  if (!table) {
    std::cerr << "EE: point of interest file " << poi_file << " is not valid" << "\n";
    exit(1);
  }
  std::vector<std::string> benchmark = table->Column("benchmark");
  std::vector<std::string> file = table->Column("file");
  std::vector<std::string> linum = table->Column("linum");
  // the type field is needed. But if it is not correct, Helium will simply emit an error and continue next POI
  std::vector<std::string> type = table->Column("type");
  // this field is optional. If not provided, the failure condition will be empty
  std::vector<std::string> fc = table->Column("failure-condition");

  if (benchmark.empty()
      || file.empty()
      || linum.empty()
      || type.empty()
      // || fc.empty()
      ) {
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
      std::string fcstr=fc.empty()?"":fc[i];
      ret.push_back(new PointOfInterest({folder, file[i], stoi(linum[i]), type[i], fcstr}));
    }
  }
  return ret;
}

