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

struct DataFrameReference {
  // Must always go from uint64_t --> uint64_t
  enum class CompressionType { kSimple8b, kZigZag, kDeltaZigZag, kRLE2, kRLE4 };
  DataFrameReference(uint min, uint max, uint resolution_bits)
      : min(min), max(max), resolution_bits(resolution_bits) {}
  uint min, max;
  uint8_t resolution_bits;
  std::vector<CompressionType> value_compressions;
  std::vector<CompressionType> time_compressions;
  std::chrono::time_point<std::chrono::system_clock> start_time_utc;
  std::chrono::time_point<std::chrono::steady_clock> start_time_monotonic;
};

union CompressionSideChannel {
  uint64_t initial_value;
  uint64_t initial_time;
  size_t simple8b_uncompressed_size;
  size_t rle_uncompressed_size;
};

struct CompressedDataFrame {
  float value_compression_ratio = 0;
  float time_compression_ratio = 0;
  std::vector<CompressionSideChannel> side_channel;
  std::vector<uint8_t> values;
  std::vector<uint8_t> times;
};

class DataFrame {
 public:
  void Record(steady_time_point_t, uint value);
  const std::vector<uint>& Values() const;

  // TODO
  // Breaks if you do > 1 simple8b in a row
  // Time compression is not implemented
  std::optional<CompressedDataFrame> Compress(const DataFrameReference& reference) const;
  static std::optional<DataFrame> Decompress(const DataFrameReference& reference,
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
  static bool Decompress(DataFrameReference::CompressionType compression_type,
                         CompressionMemory& mem,
                         std::optional<CompressionSideChannel>& side_channel);
  bool Compress(DataFrameReference::CompressionType compression_type, CompressionMemory& mem,
                std::optional<CompressionSideChannel>& side_channel) const;

  std::vector<steady_time_point_t> times_;
  std::vector<uint> values_;
};

}  // namespace sensor_compress