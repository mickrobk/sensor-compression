#include "example_time_sensor.h"

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <thread>

#include "json.h"
#include "time_util.h"

namespace sensor_compress {
namespace {

tl::expected<ExampleTimeSensor, std::string> GetWithValues() {
  ExampleTimeSensor sensor;
  for (int i = 0; i < 10; ++i) {
    auto r = sensor.Update(steady_clock_now());
    if (!r) return tl::unexpected(r.error());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return sensor;
}

TEST(ExampleTimeSensorTest, Json) {
  auto sensor = GetWithValues();
  ASSERT_TRUE(sensor);
  auto readings = sensor->TakeReadings(true);
  ASSERT_TRUE(readings);

  Json j(*readings);
  // fmt::println("{}", j.dump());
  auto compressed_readings = j.get<CompressedSensorReadings>();
  auto decompressed_readings = compressed_readings.Decompress();
  ASSERT_TRUE(decompressed_readings);
  // fmt::println("-----");
  // fmt::println("{}", Json(*decompressed_readings).dump());
}

}  // namespace
}  // namespace sensor_compress
