#pragma once

#include <json.h>

#include "sensor.inl"

#include <cstdint>
#include <string>
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
};

template <typename TString = std::string>
class Sensor {
 protected:
  Sensor(DataHeader header, CombinedCorrection correction = CombinedCorrection())
      : header_(header), correction_(std::move(correction)) {}

 public:
  virtual ~Sensor() = default;

  tl::expected<void, std::string> Update(steady_time_point_t t) {
    if (current_frame_.size() >= header_.frame_size) {
      auto compressed_current = current_frame_.Compress(header_);
      current_frame_.Clear();
      if (!compressed_current) return tl::unexpected{compressed_current.error()};
      compressed_values_.push_back(std::move(compressed_current.value()));
    }

    auto value = GetValue(t);
    if (!value) return {};

    current_frame_.Record(*value);
    last_ = *value;
    return {};
  }
  std::optional<DataFrameValue> GetLast() const { return last_; }

  CompressedSensorReadings TakeReadings() {
    CompressedSensorReadings readings;
    readings.header = header_;
    readings.frames = std::move(compressed_values_);
    compressed_values_.clear();
    return readings;
  }

 private:
 protected:
  virtual std::optional<DataFrameValue> GetValue(steady_time_point_t t) = 0;

  DataFrameValue last_;
  DataHeader header_;
  CombinedCorrection correction_;
  DataFrame current_frame_;
  std::vector<CompressedDataFrame> compressed_values_;
};
}  // namespace sensor_compress
