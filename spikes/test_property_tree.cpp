#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>
namespace pt = boost::property_tree;

struct config
{
    std::string m_file;               // config filename
    std::string experiment; // experiment name
    std::set<std::string> m_modules;  // modules where logging is enabled
    void load(const std::string &filename);
    void save(const std::string &filename);
};

int main() {}
