#include <gtest/gtest.h>
#include "utils/utils.h"

int main(int argc, char** argv) {
  utils::seed_rand();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
