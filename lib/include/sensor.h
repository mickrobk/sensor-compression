#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
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

  tl::expected<SensorReadings, std::string> Decompress() const;
};

class Sensor {
 protected:
  explicit Sensor(DataHeader header) : header_(header) {}

 public:
  virtual ~Sensor() = default;
  void SetSessionId(uuids::uuid id) {
    header_.session_id = id;
    header_.intra_session_id = UuidGen::Generate();
  }
  std::string_view Name() const { return header_.name; }
  uuids::uuid SessionId() const { return header_.session_id; }
  uuids::uuid IntraSessionId() const { return header_.intra_session_id; }

  tl::expected<void, std::string> Update(steady_time_point_t t);
  std::optional<DataFrameValue> GetLast() const { return last_; }

  // threadsafe
  tl::expected<CompressedSensorReadings, std::string> TakeHistory(bool flush = true);

 protected:
  virtual std::optional<DataFrameValue> GetValue(steady_time_point_t t) = 0;
  const std::vector<CompressedDataFrame>& CompressedValues() const {
    std::lock_guard lock(mutex_);
    return compressed_values_;
  }
  tl::expected<void, std::string> MaybeCompressCurrent(bool force);

 private:
  DataFrameValue last_;
  DataHeader header_;
  DataFrame current_frame_;

  mutable std::mutex mutex_;
  std::vector<CompressedDataFrame> compressed_values_;
};

}  // namespace sensor_compress
