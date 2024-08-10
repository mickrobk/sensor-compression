#include "sensor.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using namespace sensor_compress;

class TestSensor : public Sensor<std::string> {
 public:
  TestSensor(DataHeader header, CombinedCorrection correction = CombinedCorrection())
      : Sensor(header, std::move(correction)) {}

  const std::vector<std::string>& CompressedValues() const { return compressed_values_; }

  void SetNextValue(uint value) { value_ = value; }

 protected:
  std::optional<DataFrameValue> GetValue(steady_time_point_t timestamp) override {
    return DataFrameValue{timestamp, value_};
  }
  uint value_ = 0;
};

TEST(SensorTest, UpdateTest) {
  DataHeader header(0, 100);
  TestSensor sensor(header);

  sensor.SetNextValue(3);
  auto result = sensor.Update(SteadyFromMs(0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(sensor.GetLast()->value, 3);
  EXPECT_EQ(ToMs(sensor.GetLast()->t), 0);
  EXPECT_EQ(sensor.CompressedValues().size(), 0);

  for (int i = 0; i < header.frame_size; ++i) {
    sensor.SetNextValue(3 + i);
    sensor.Update(SteadyFromMs(i));
  }

  EXPECT_EQ(sensor.CompressedValues().size(), 1);
  nlohmann::json json_result = nlohmann::json::parse(sensor.CompressedValues().back());
  EXPECT_TRUE(json_result.contains("values"));
  EXPECT_TRUE(json_result.contains("times"));

  // Decompress json_result using DataFrame::Decompress
  DataFrame frame;
  frame.Record(SteadyFromMs(0), 0);  // Initialize with dummy data
  auto decompressed_frame = DataFrame::Decompress(header, json_result);
  ASSERT_TRUE(decompressed_frame.has_value());
  const auto& decompressed_values = decompressed_frame->Values();
  const auto& decompressed_times = decompressed_frame->Times();

  // Verify decompressed values
  for (int i = 0; i < header.frame_size; ++i) {
    EXPECT_EQ(decompressed_values[i], 3 + i);
    EXPECT_EQ(decompressed_times[i], i);
  }
}