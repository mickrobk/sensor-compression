#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "simple8b.h"

TEST(CompressTest, delta) {
  int64_t foo[] = {1, 2, 3, 2, 12};
  DeltaEncode(foo, 5);
  EXPECT_THAT(foo, ::testing::ElementsAre(1, 1, 1, -1, 10));
  DeltaEncode(foo, 5);
  EXPECT_THAT(foo, ::testing::ElementsAre(1, 0, 0, -2, 11));
  DeltaDecode(foo, 5);
  EXPECT_THAT(foo, ::testing::ElementsAre(1, 1, 1, -1, 10));
  DeltaDecode(foo, 5);
  EXPECT_THAT(foo, ::testing::ElementsAre(1, 2, 3, 2, 12));
}

TEST(CompressTest, zigzag) {
  int64_t foo[] = {0, 1, 2, 4, -2, -1};
  ZigZagEncode(foo, 6);
  EXPECT_THAT(foo, ::testing::ElementsAre(0, 2, 4, 8, 3, 1));
  ZigZagDecode(foo, 6);
  EXPECT_THAT(foo, ::testing::ElementsAre(0, 1, 2, 4, -2, -1));
}