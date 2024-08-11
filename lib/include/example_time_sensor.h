#pragma once

#include "sensor.h"
#include "time_util.h"

namespace sensor_compress {

class ExampleTimeSensor : public Sensor {
 public:
  static DataHeader StandardHeader();
  explicit ExampleTimeSensor(DataHeader header = StandardHeader());

 protected:
  std::optional<DataFrameValue> GetValue(steady_time_point_t t) override;

 private:
  steady_time_point_t startup_time_;
};

}  // namespace sensor_compress
