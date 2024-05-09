#include "sensor.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace sensor_compress;

TEST(FPointwiseCorrectionTest, noref) {
  PointwiseCorrection pc({});
  EXPECT_FLOAT_EQ(0, pc.Correct(0));
  EXPECT_FLOAT_EQ(1, pc.Correct(1));
  EXPECT_FLOAT_EQ(-1, pc.Correct(-1));
}

TEST(FPointwiseCorrectionTest, oneref) {
  PointwiseCorrection pc({{0, 10}});
  EXPECT_FLOAT_EQ(-1, pc.Correct(-1));
  EXPECT_FLOAT_EQ(10, pc.Correct(0));
  EXPECT_FLOAT_EQ(1, pc.Correct(1));
}

TEST(FPointwiseCorrectionTest, tworef) {
  PointwiseCorrection pc({{0, 10}, {1, 20}});
  EXPECT_FLOAT_EQ(-1, pc.Correct(-1));
  EXPECT_FLOAT_EQ(-.1, pc.Correct(-.1));
  EXPECT_FLOAT_EQ(10, pc.Correct(0));
  EXPECT_FLOAT_EQ(11, pc.Correct(0.1));
  EXPECT_FLOAT_EQ(15, pc.Correct(.5));
  EXPECT_FLOAT_EQ(18, pc.Correct(.8));
  EXPECT_FLOAT_EQ(20, pc.Correct(1));
}

TEST(ILinearCorrectionTest, linear) {
  LinearCorrection ic(0, 10, 10, 30);
  EXPECT_FLOAT_EQ(ic.Correct(0), 10);
  EXPECT_FLOAT_EQ(ic.Correct(5), 20);
  EXPECT_FLOAT_EQ(ic.Correct(10), 30);
}
TEST(ILinearCorrectionTest, outOfBounds) {
  LinearCorrection ic(5, 10, 10, 30);
  EXPECT_FLOAT_EQ(ic.Correct(0), -10);
  EXPECT_FLOAT_EQ(ic.Correct(5), 10);
  EXPECT_FLOAT_EQ(ic.Correct(15), 50);
}