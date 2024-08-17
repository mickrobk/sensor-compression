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
  auto readings = sensor->TakeHistory(true);
  ASSERT_TRUE(readings);

  Json j(*readings);
  fmt::println("{}", j.dump());
  auto compressed_readings = j.get<CompressedSensorReadings>();
  auto decompressed_readings = compressed_readings.Decompress();
  ASSERT_TRUE(decompressed_readings);
  fmt::println("-----");
  fmt::println("{}", Json(*decompressed_readings).dump());

  for (auto& frame : compressed_readings.frames) {
    std::string buf0 = base64::encode_into<std::string>(frame.times.begin(), frame.times.end());
    std::string buf = base64::encode_into<std::string>(frame.values.begin(), frame.values.end());
    auto sc_bytes = to_bytes(frame.side_channel);
    std::string buf2 = base64::encode_into<std::string>(sc_bytes.begin(), sc_bytes.end());

    fmt::println("b64-times: {}", buf0);
    fmt::println("b64-values: {}", buf);
    fmt::println("b64-sc: {}", buf2);
  }
}

}  // namespace
}  // namespace sensor_compress
