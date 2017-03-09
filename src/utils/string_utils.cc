#include "helium/utils/string_utils.h"
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "helium/utils/common.h"

/*******************************
 ** string utils
 *******************************/

namespace utils {


  // gtest document says the test_case_name and test_name should not contain _
  TEST(utils_test_case, trim_test) {
    std::string s = " ab cd  ";
    trim(s);
    EXPECT_EQ(s, "ab cd");
    s = "\tabcd\t   \n";
    trim(s);
    EXPECT_EQ(s, "abcd");
    s = "   \t\n";
    trim(s);
    EXPECT_TRUE(s.empty());
  }



  /**
   * spilt string based, delimiter is *space*
   * @param s [in] string to be spliteed. Not modified.
   * @return a vector of string
   */
  std::vector<std::string>
  split(const std::string &s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens{
      std::istream_iterator<std::string>{iss},
        std::istream_iterator<std::string>{}
    };
    return tokens;
  }

  TEST(utils_test_case, split_test) {
    std::string s = "  ab  cd \n \t ef";
    std::vector<std::string> vs = split(s);
    EXPECT_FALSE(vs.empty());
    EXPECT_EQ(vs.size(), 3);
    EXPECT_EQ(vs[0], "ab");
    EXPECT_EQ(vs[1], "cd");
    EXPECT_EQ(vs[2], "ef");
    vs = split("hello= ", '=');
    ASSERT_EQ(vs.size(), 2);
    vs = split("hello=", '=');
    ASSERT_EQ(vs.size(), 2);
  }

  /**
   * split string based on delimeter given
   * @param[in] s string to be split
   * @param[in] delim delimeter
   * @return elems store the splits
   */
  std::vector<std::string>
  split(const std::string &s, char delim) {
    std::vector<std::string> ret;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      ret.push_back(item);
    }
    // in the case the last one is the delim, push an empty string.
    // This is useful for csv file
    if (!s.empty() && s.back() == delim) {
      ret.push_back("");
    }
    return ret;
  }

  TEST(UtilsTestCase, SplitTest) {
    std::vector<std::string> result;
    result = split("|Hello|World|", '|');
    ASSERT_EQ(result.size(), 4);
  }

  /**
   * Delim by ANY characters in delim string
   */
  std::vector<std::string>
  split(std::string s, std::string delim) {
    std::size_t prev = 0, pos;
    std::vector<std::string> ret;
    while ((pos = s.find_first_of(delim, prev)) != std::string::npos)
      {
        if (pos > prev)
          ret.push_back(s.substr(prev, pos-prev));
        prev = pos+1;
      }
    if (prev < s.length()) {
      ret.push_back(s.substr(prev, std::string::npos));
    }
    return ret;
  }


  /**
   * Split by the delim as a whole
   */
  std::vector<std::string>
  split_by_string(std::string s, std::string delim) {
    std::size_t prev = 0, pos;
    std::vector<std::string> ret;
    while ((pos = s.find(delim, prev)) != std::string::npos) {
      if (pos > prev)
        ret.push_back(s.substr(prev, pos-prev));
      prev = pos+delim.size();
    }
    if (prev < s.length()) {
      ret.push_back(s.substr(prev, std::string::npos));
    }
    return ret;
  }

  TEST(UtilsCase, SplitByStringTest) {
    std::vector<std::string> v = split_by_string("hello world 123 hello world", "or");
    ASSERT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], "hello w");
    EXPECT_EQ(v[1], "ld 123 hello w");
    EXPECT_EQ(v[2], "ld");
  }


  /**
   * If string s ends with string pattern.
   */
  bool ends_with(const std::string &s, const std::string &pattern) {
    if (s.length() >= pattern.length()) {
      return (0 == s.compare (s.length() - pattern.length(), pattern.length(), pattern));
    } else {
      return false;
    }
  }
  /**
   * If string s starts with string pattern.
   */
  bool starts_with(const std::string &s, const std::string &pattern) {
    if (s.length() >= pattern.length()) {
      return (0 == s.compare (0, pattern.length(), pattern));
    } else {
      return false;
    }
  }

  /**
   * remove all currence of pattern from s
   */
  void remove(std::string& s, const std::string& pattern) {
    while (s.find(pattern) != std::string::npos) {
      s.erase(s.find(pattern), pattern.length());
    }
  }

  /**
   * Replace all occurrence of pattern with replacement, in the input/output string s.
   */
  void replace(std::string &s, std::string pattern, std::string replacement) {
    while (s.find(pattern) != std::string::npos) {
      s.replace(s.find(pattern), pattern.length(), replacement);
    }
  }
  std::string indent_string(std::string raw, int indent) {
    assert(indent >= 0);
    std::string indention = std::string("  ", indent);
    std::vector<std::string> lines = utils::split(raw, '\n');
    for (std::string line : lines) {
      line = indention + line;
    }
    return boost::algorithm::join(lines, "\n");
  }
}
