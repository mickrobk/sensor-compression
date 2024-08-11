#include "sensor.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using namespace sensor_compress;

class TestSensor : public Sensor {
 public:
  TestSensor(DataHeader header) : Sensor(header) {}

  void SetNextValue(uint value) { value_ = value; }

  const std::vector<CompressedDataFrame>& TestCompressedValues() const {
    return CompressedValues();
  }

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
  EXPECT_EQ(sensor.TestCompressedValues().size(), 0);

  for (int i = 0; i < header.frame_size; ++i) {
    sensor.SetNextValue(1 + i);
    sensor.Update(SteadyFromMs(1 + i));
  }

  auto readings = sensor.TakeHistory();
  ASSERT_EQ(readings->frames.size(), 1);
  ASSERT_EQ(sensor.TestCompressedValues().size(), 0);

  const auto& compressed_frame = readings->frames.back();

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

TEST(SensorTest, DecompressTest) {
  DataHeader header(0, 100);
  header.frame_size = 10;
  header.value_compressions = DataHeader::DefaultValueCompression();
  header.time_compressions = DataHeader::DefaultTimeCompression();
  TestSensor sensor(header);

  for (int i = 0; i < header.frame_size; ++i) {
    sensor.SetNextValue(i);
    sensor.Update(SteadyFromMs(i));
  }

  auto readings = sensor.TakeHistory();
  ASSERT_EQ(readings->frames.size(), 1);

  auto decompressed = readings->Decompress();
  ASSERT_TRUE(decompressed.has_value());

  const auto& values = decompressed->values;
  ASSERT_EQ(values.size(), header.frame_size);

  for (int i = 0; i < header.frame_size; ++i) {
    EXPECT_EQ(values[i].value, i);
    EXPECT_EQ(ToMs(values[i].t), i);
  }
}

TEST(SensorTest, TakeReadingsTest) {
  DataHeader header(0, 100);
  header.frame_size = 100;
  header.value_compressions = DataHeader::DefaultValueCompression();
  header.time_compressions = DataHeader::DefaultTimeCompression();
  TestSensor sensor(header);

  for (int i = 0; i < 2 * header.frame_size; ++i) {
    sensor.SetNextValue(i);
    sensor.Update(SteadyFromMs(i));
  }

  auto readings1 = sensor.TakeHistory(true);
  ASSERT_TRUE(readings1.has_value());
  ASSERT_EQ(readings1->frames.size(), 2);
  ASSERT_EQ(sensor.TestCompressedValues().size(), 0);

  for (int i = 0; i < header.frame_size; ++i) {
    sensor.SetNextValue(i);
    sensor.Update(SteadyFromMs(i));
  }

  auto readings2 = sensor.TakeHistory(true);
  ASSERT_TRUE(readings2.has_value());
  ASSERT_EQ(readings2->frames.size(), 1);
  ASSERT_EQ(sensor.TestCompressedValues().size(), 0);
}
