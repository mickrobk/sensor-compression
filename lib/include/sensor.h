#pragma once

#include <json.h>

#include <cstdint>
#include <string>
#include <tl/expected.hpp>

#include "correction.h"
#include "data_frame.h"
#include "data_header.h"
#include "data_stream.h"

namespace sensor_compress {

class SensorReadings {
  DataHeader header;
  std::vector<DataFrameValue> values;
};

class CompressedSensorReadings {
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
    auto value = GetValue(t);
    if (!value) return {};

    last_ = *value;
    auto maybe_compressed = stream_.Record(header_, *value);
    if (maybe_compressed) {
      auto compressed = std::move(maybe_compressed.value());
      if (!compressed) return tl::unexpected{compressed.error()};
      sensor_compress::CompressedDataFrame frame = std::move(compressed.value());
      nlohmann::json json_result(frame);
      compressed_values_.push_back(std::move(frame));
    }
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

 protected:
  virtual std::optional<DataFrameValue> GetValue(steady_time_point_t t) = 0;

  DataFrameValue last_;
  DataHeader header_;
  CombinedCorrection correction_;
  DataStream stream_;
  std::vector<CompressedDataFrame> compressed_values_;
};
}  // namespace sensor_compress
