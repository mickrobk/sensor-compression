#include "example_time_sensor.h"

#include <gtest/gtest.h>

#include <thread>

#include "time_util.h"

namespace sensor_compress {
namespace {

TEST(ExampleTimeSensorTest, Update) {
  ExampleTimeSensor sensor;

  auto start_time = steady_clock_now();
  auto update_result = sensor.Update(start_time);
  EXPECT_TRUE(update_result.has_value());

  auto last_value = sensor.GetLast();
  ASSERT_TRUE(last_value.has_value());
  EXPECT_EQ(last_value->t, start_time);
  EXPECT_EQ(last_value->value, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  update_result = sensor.Update(steady_clock_now());
  EXPECT_TRUE(update_result.has_value());

  last_value = sensor.GetLast();
  ASSERT_TRUE(last_value.has_value());
  EXPECT_GT(last_value->value, 0);
}

}  // namespace
}  // namespace sensor_compress
