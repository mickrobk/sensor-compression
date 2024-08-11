#include "example_time_sensor.h"

namespace sensor_compress {

ExampleTimeSensor::ExampleTimeSensor(DataHeader header) : Sensor(header) {
  startup_time_ = steady_clock_now();
}

std::optional<DataFrameValue> ExampleTimeSensor::GetValue(steady_time_point_t t) {
  return DataFrameValue{t, static_cast<uint32_t>(ToMs(steady_clock_now()) - ToMs(startup_time_))};
}

}  // namespace sensor_compress
