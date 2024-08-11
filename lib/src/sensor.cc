
#include "sensor.h"

#include <string>
#include <tl/expected.hpp>
#include <vector>
namespace sensor_compress {

tl::expected<void, std::string> Sensor::MaybeCompressCurrent(bool force) {
  if ((force && current_frame_.size() > 0) || current_frame_.size() >= header_.frame_size) {
    auto compressed_current = current_frame_.Compress(header_);
    current_frame_.Clear();
    if (!compressed_current) return tl::unexpected{compressed_current.error()};
    compressed_values_.push_back(std::move(compressed_current.value()));
    header_.SetTimeToNow();
  }
  return {};
}

tl::expected<void, std::string> Sensor::Update(steady_time_point_t t) {
  auto value = GetValue(t);
  if (!value) return {};

  current_frame_.Record(*value);
  last_ = *value;

  auto compress_result = MaybeCompressCurrent(/*force=*/false);
  if (!compress_result) return compress_result;

  return {};
}

tl::expected<CompressedSensorReadings, std::string> Sensor::Take(bool flush) {
  if (flush) {
    auto compress_result = MaybeCompressCurrent(true);
    if (!compress_result) {
      return tl::unexpected{compress_result.error()};
    }
  }
  CompressedSensorReadings readings;
  readings.header = header_;
  readings.frames = std::move(compressed_values_);
  compressed_values_.clear();
  return readings;
}

tl::expected<SensorReadings, std::string> CompressedSensorReadings::Decompress() const {
  SensorReadings result;
  result.header = header;

  for (const auto& frame : frames) {
    auto decompressed = DataFrame::Decompress(header, frame);
    if (!decompressed) {
      return tl::unexpected{decompressed.error()};
    }
    const auto& v = decompressed->Values();
    const auto& t = decompressed->Times();
    for (size_t i = 0; i < v.size(); ++i) {
      result.values.push_back({t[i], v[i]});
    }
  }

  return result;
}

}  // namespace sensor_compress
