#pragma once

#include <cstdint>
#include <string>
#include <algorithm>
#include <tl/expected.hpp>

#include "correction.h"
#include "data_frame.h"
#include "data_header.h"

namespace sensor_compress {

struct SensorReadings {
  DataHeader header;
  std::vector<DataFrameValue> values;
};

struct CompressedSensorReadings {
  DataHeader header;
  std::vector<CompressedDataFrame> frames;

  SensorReadings Decompress() const;
};

class Sensor {
 protected:
  Sensor(DataHeader header, CombinedCorrection correction = CombinedCorrection())
      : header_(header), correction_(std::move(correction)) {}

 public:
  virtual ~Sensor() = default;

  tl::expected<void, std::string> Update(steady_time_point_t t);
  std::optional<DataFrameValue> GetLast() const { return last_; }

  CompressedSensorReadings TakeReadings();

 protected:
  virtual std::optional<DataFrameValue> GetValue(steady_time_point_t t) = 0;
  const std::vector<CompressedDataFrame>& CompressedValues() const { return compressed_values_; }

 private:
  DataFrameValue last_;
  DataHeader header_;
  CombinedCorrection correction_;
  DataFrame current_frame_;
  std::vector<CompressedDataFrame> compressed_values_;
};

}  // namespace sensor_compress
