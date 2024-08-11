#include "example_time_sensor.h"

namespace sensor_compress {

ExampleTimeSensor::ExampleTimeSensor(DataHeader header) : Sensor(header) {
  startup_time_ = steady_time_point_t::now();
}

std::optional<DataFrameValue> ExampleTimeSensor::GetValue(steady_time_point_t t) {
  return {t, ToMs(steady_time_point_t::now() - startup_time_)};
}

}  // namespace sensor_compress
