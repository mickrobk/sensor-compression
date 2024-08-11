#pragma once

#include "sensor.h"

namespace sensor_compress {

class ExampleTimeSensor : public Sensor {
 public:
  ExampleTimeSensor(DataHeader header);

 protected:
  std::optional<DataFrameValue> GetValue(steady_time_point_t t) override;
};

}  // namespace sensor_compress
