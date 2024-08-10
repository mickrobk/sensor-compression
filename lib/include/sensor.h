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

template <typename TString = std::string>
class Sensor {
 protected:
  Sensor(DataHeader header, CombinedCorrection correction = CombinedCorrection())
      : header_(header), correction_(std::move(correction)) {}

 public:
  virtual ~Sensor() = default;
  tl::expected<void, std::string> Update(steady_time_point_t timestamp) {
    auto value = GetValue(timestamp);
    if (!value) return {};

    last_ = *value;
    auto maybe_compressed = stream_.Record(header_, *value);
    if (maybe_compressed) {
      auto compressed = std::move(maybe_compressed.value());
      if (!compressed) return tl::unexpected{compressed.error()};

      nlohmann::json json_result(*compressed);
      compressed_values_.push_back(TString(json_result.dump()));
    }
    return {};
  }
  std::optional<DataFrameValue> GetLast() const { return last_; }

 protected:
  virtual std::optional<DataFrameValue> GetValue(steady_time_point_t timestamp) = 0;

  DataFrameValue last_;
  DataHeader header_;
  CombinedCorrection correction_;
  DataStream stream_;
  std::vector<TString> compressed_values_;
};
}  // namespace sensor_compress
