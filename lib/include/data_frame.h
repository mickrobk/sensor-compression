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
#include "log_container.h"
#include "time_util.h"

namespace sensor_compress {

static inline unsigned int CountBits(uint v) {
  unsigned int count = 0;
  while (v > 0) {
    count++;
    v >>= 1;
  }
  return count;
}

struct DataHeader {
  enum class CompressionType { kSimple8b, kZigZag, kDeltaZigZag, kRLE2, kRLE4 };
  DataHeader(uint min, uint max, uint resolution_bits)
      : min(min), max(max), resolution_bits(resolution_bits) {
    start_time_utc = std::chrono::system_clock::now();
    start_time_steady = std::chrono::steady_clock::now();
  }
  DataHeader(uint min, uint max) : DataHeader(min, max, CountBits(max - min)) {}
  std::string source_identifier;
  std::string source_uuid;
  std::string data_type;
  uint32_t version = 1;
  uint min, max;
  uint8_t resolution_bits;
  size_t frame_size = 10000;
  std::vector<CompressionType> value_compressions;
  std::vector<CompressionType> time_compressions;
  utc_time_point_t start_time_utc;
  steady_time_point_t start_time_steady;
};

using CompressionSideChannel = std::variant<uint64_t, size_t>;

// union CompressionSideChannel {
//   uint64_t initial_value;
//   uint64_t initial_time;
//   size_t simple8b_uncompressed_size;
//   size_t rle_uncompressed_size;
// };

struct CompressedDataFrame {
  std::vector<CompressionSideChannel> side_channel;
  std::vector<uint8_t> values;
  std::vector<uint8_t> times;

  bool operator==(const CompressedDataFrame& other) const {
    return side_channel == other.side_channel && values == other.values && times == other.times;
  }

  std::vector<uint8_t> Serialize() const;
  static std::optional<CompressedDataFrame> Deserialize(const std::vector<uint8_t>& data);
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
                         std::optional<CompressionSideChannel>& side_channel);
  bool Compress(DataHeader::CompressionType compression_type, CompressionMemory& mem,
                std::optional<CompressionSideChannel>& side_channel) const;

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
  std::optional<DataFrame> working_frame_;
  std::optional<size_t> working_offset_into_past_ = 0;
};

}  // namespace sensor_compress