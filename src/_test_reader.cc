#include "reader.h"
#include <gtest/gtest.h>
#include "config.h"

TEST(reader_test_case, DISABLED_divide_segment) {
  Config::Instance()->ParseFile("./helium.conf");
  Config::Instance()->Set("code-selection-method", "divide");
  char tmp_dir[] = "/tmp/helium-test-temp.XXXXXX";
  char *result = mkdtemp(tmp_dir);
  ASSERT_TRUE(result != NULL);
  std::string dir = tmp_dir;
  std::string filename = dir+"/a.c";
  const char *code = R"prefix(

int main() {
  int sum = 0;
  int a = 2;
  for (int i=0;i<100;i++) {
    sum += i;
  }
  int b = 3;
  // this is a comment
  b ++;
  int d = 9;
  return 0;
}


)prefix";
  utils::write_file(filename, code);
  // create reader
  Reader *reader = new Reader(filename);
  // 4+3+2+1 + 1 = 11
  EXPECT_EQ(reader->GetSegmentCount(), 11);
  delete reader;
  
  code = R"prefix(

int main() {
  int sum = 0;
  int a = 2;
  for (int i=0;i<100;i++) {
    sum += i;
  }
}
)prefix";
  utils::write_file(filename, code);
  // create reader
  reader = new Reader(filename);
  EXPECT_EQ(reader->GetSegmentCount(), 4);
  delete reader;

}
