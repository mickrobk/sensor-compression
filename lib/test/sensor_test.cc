#include "sensor.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using namespace sensor_compress;

class TestSensor : public Sensor<std::string> {
 public:
  TestSensor(DataHeader header, CombinedCorrection correction = CombinedCorrection())
      : Sensor(header, std::move(correction)) {}

  const std::vector<std::string>& CompressedValues() const { return compressed_values_; }

 protected:
  std::optional<DataFrameValue> GetValue() override {
    static uint value = 0;
    return DataFrameValue{steady_time_point_t(std::chrono::milliseconds(value++)), value};
  }
};

TEST(SensorTest, UpdateTest) {
  DataHeader header(0, 100);
  TestSensor sensor(header);

  auto result = sensor.Update();
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(sensor.GetLast()->value, 1);
  EXPECT_EQ(sensor.CompressedValues().size(), 0);

  for (int i = 0; i < header.frame_size; ++i) {
    sensor.Update();
  }

  EXPECT_EQ(sensor.CompressedValues().size(), 1);
  nlohmann::json json_result = nlohmann::json::parse(sensor.CompressedValues().back());
  EXPECT_TRUE(json_result.contains("values"));
  EXPECT_TRUE(json_result.contains("times"));
}