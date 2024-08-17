#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "time_util.h"
#include "uuid_gen.h"

namespace sensor_compress {

struct DataHeader {
  enum class CompressionType : uint8_t {
    kSimple8b = 0,     //
    kZigZag = 1,       //
    kDeltaZigZag = 2,  //
    kRLE2 = 3,         //
    kRLE4 = 4          //
  };

  static const std::vector<CompressionType> DefaultValueCompression() {
    return {DataHeader::CompressionType::kDeltaZigZag, DataHeader::CompressionType::kRLE2,
            DataHeader::CompressionType::kSimple8b};
  }
  static const std::vector<CompressionType> DefaultTimeCompression() {
    return {DataHeader::CompressionType::kDeltaZigZag, DataHeader::CompressionType::kRLE2,
            DataHeader::CompressionType::kSimple8b};
  }

  DataHeader();
  DataHeader(uint min, uint max, uint resolution_bits);
  DataHeader(uint min, uint max);
  void SetTimeToNow();

  uuids::uuid session_id = UuidGen::Generate();
  std::string name;
  uint32_t version = 1;
  uint min, max;
  uint8_t resolution_bits;
  size_t frame_size = 10000;
  std::vector<CompressionType> value_compressions;
  std::vector<CompressionType> time_compressions;
  utc_time_point_t start_time_utc;
  steady_time_point_t start_time_steady;
};

}  // namespace sensor_compress
