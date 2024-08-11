#include "example_time_sensor.h"

namespace sensor_compress {

DataHeader ExampleTimeSensor::StandardHeader() {
  DataHeader header(0, std::pow(2, 32) - 1, 32);
  header.name = "example time sensor";
  header.value_compressions = DataHeader::DefaultValueCompression();
  header.time_compressions = DataHeader::DefaultTimeCompression();
  header.frame_size = 1000;
  return header;
}

ExampleTimeSensor::ExampleTimeSensor(DataHeader header) : Sensor(header) {
  startup_time_ = steady_clock_now();
}

std::optional<DataFrameValue> ExampleTimeSensor::GetValue(steady_time_point_t t) {
  return DataFrameValue{t, static_cast<uint32_t>(ToMs(steady_clock_now()) - ToMs(startup_time_))};
}

}  // namespace sensor_compress
