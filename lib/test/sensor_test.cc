#include "sensor.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using namespace sensor_compress;

class TestSensor : public Sensor<std::string> {
 public:
  TestSensor(DataHeader header, CombinedCorrection correction = CombinedCorrection())
      : Sensor(header, std::move(correction)) {}

  const std::vector<CompressedDataFrame>& CompressedValues() const { return compressed_values_; }  

  void SetNextValue(uint value) { value_ = value; }

 protected:
  std::optional<DataFrameValue> GetValue(steady_time_point_t timestamp) override {
    return DataFrameValue{timestamp, value_};
  }
  uint value_ = 0;
};

TEST(SensorTest, UpdateTest) {
  DataHeader header(0, 100);
  header.frame_size = 100;
  header.value_compressions = DataHeader::DefaultValueCompression();
  header.time_compressions = DataHeader::DefaultTimeCompression();
  TestSensor sensor(header);

  sensor.SetNextValue(0);
  auto result = sensor.Update(SteadyFromMs(0));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(sensor.GetLast()->value, 0);
  EXPECT_EQ(ToMs(sensor.GetLast()->t), 0);
  EXPECT_EQ(sensor.CompressedValues().size(), 0);

  for (int i = 0; i < header.frame_size; ++i) {
    sensor.SetNextValue(1 + i);
    sensor.Update(SteadyFromMs(1 + i));
  }

  ASSERT_EQ(sensor.CompressedValues().size(), 1);
  const auto& compressed_frame = sensor.CompressedValues().back();

  // Decompress compressed_frame using DataFrame::Decompress  
  auto decompressed_frame = DataFrame::Decompress(header, compressed_frame);
  ASSERT_TRUE(decompressed_frame);
  const auto& decompressed_values = decompressed_frame->Values();
  const auto& decompressed_times = decompressed_frame->Times();

  // Verify decompressed values
  for (int i = 0; i < header.frame_size; ++i) {
    EXPECT_EQ(decompressed_values[i], i);
    EXPECT_EQ(ToMs(decompressed_times[i]), i);
  }
}
