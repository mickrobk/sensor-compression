#include <gtest/gtest.h>

#include "example_time_sensor.h"

namespace sensor_compress {
namespace {

TEST(ExampleTimeSensorTest, GetValue) {
  DataHeader header;
  header.frame_size = 10;
  ExampleTimeSensor sensor(header);

  auto start_time = steady_time_point_t::now();
  auto value1 = sensor.GetValue(start_time);
  ASSERT_TRUE(value1.has_value());
  EXPECT_EQ(value1->timestamp, start_time);
  EXPECT_EQ(value1->value, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  auto value2 = sensor.GetValue(steady_time_point_t::now());
  ASSERT_TRUE(value2.has_value());
  EXPECT_GT(value2->value, 0);
}

TEST(ExampleTimeSensorTest, Update) {
  DataHeader header;
  header.frame_size = 10;
  ExampleTimeSensor sensor(header);

  auto start_time = steady_time_point_t::now();
  auto update_result = sensor.Update(start_time);
  EXPECT_TRUE(update_result.has_value());

  auto last_value = sensor.GetLast();
  ASSERT_TRUE(last_value.has_value());
  EXPECT_EQ(last_value->timestamp, start_time);
  EXPECT_EQ(last_value->value, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  update_result = sensor.Update(steady_time_point_t::now());
  EXPECT_TRUE(update_result.has_value());

  last_value = sensor.GetLast();
  ASSERT_TRUE(last_value.has_value()); 
  EXPECT_GT(last_value->value, 0);
}

}  // namespace
}  // namespace sensor_compress
