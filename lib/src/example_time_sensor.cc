#include "example_time_sensor.h"

namespace sensor_compress {

ExampleTimeSensor::ExampleTimeSensor(DataHeader header) : Sensor(header) {}

std::optional<DataFrameValue> ExampleTimeSensor::GetValue(steady_time_point_t t) {
  // TODO: Implement logic to return sensor value at time t
  return std::nullopt;
}

}  // namespace sensor_compress  
