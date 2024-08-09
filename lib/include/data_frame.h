
#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include "compress.h"
#include "correction.h"
#include "data_header.h"
#include "log_container.h"
#include "time_util.h"

namespace sensor_compress {

struct CompressedDataFrame {
  std::vector<uint64_t> side_channel;
  std::vector<uint8_t> values;
  std::vector<uint8_t> times;
};

struct DataFrameValue {
  steady_time_point_t t;
  uint value;
};

class DataFrame {
 public:
  void Record(DataFrameValue value) { Record(value.t, value.value); }
  void Record(steady_time_point_t, uint value);
  void Clear() {
    times_.clear();
    values_.clear();
  }
  const std::vector<uint>& Values() const;
  const std::vector<steady_time_point_t>& Times() const;
  size_t size() const { return times_.size(); }

  // Breaks if you do > 1 simple8b in a row
  std::optional<CompressedDataFrame> Compress(const DataHeader& reference,
                                              float* value_compression_ratio = nullptr,
                                              float* time_compression_ratio = nullptr) const;
  static std::optional<DataFrame> Decompress(const DataHeader& reference,
                                             const CompressedDataFrame& data);

 private:
  struct CompressionMemory {
    std::vector<uint64_t> ua, ub;
    std::vector<int32_t> sa;
    size_t size;
    explicit CompressionMemory(size_t size) { resize(size); }
    void resize(size_t size) {
      this->size = size;
      ua.resize(size);
      ub.resize(size);
      sa.resize(size);
    }
  };
  static bool Decompress(DataHeader::CompressionType compression_type, CompressionMemory& mem,
                         std::optional<uint64_t>& side_channel);
  bool Compress(DataHeader::CompressionType compression_type, CompressionMemory& mem,
                std::optional<uint64_t>& side_channel) const;

  std::vector<steady_time_point_t> times_;
  std::vector<uint> values_;
};

class DataStream {
 public:
  bool Record(const DataHeader& header, DataFrameValue value) {
    if (current_frame_.size() >= header.frame_size) {
      if (auto compressed = current_frame_.Compress(header)) {
        past_frames_.push_back(*std::move(compressed));
        current_frame_.Clear();
        return true;
      } else {
        printf("Failed to compress current frame\n");
        return false;
      }
    }
    return false;
  }
  bool Record(const DataHeader& header, uint value) {
    return Record(header, {std::chrono::steady_clock::now(), value});
  }

 private:
  DataFrame current_frame_;
  std::vector<CompressedDataFrame> past_frames_;
};

}  // namespace sensor_compress
