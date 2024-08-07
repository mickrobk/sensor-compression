#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "time_util.h"

namespace sensor_compress {

struct DataHeader {
  enum class CompressionType {
    kSimple8b = 0,     //
    kZigZag = 1,       //
    kDeltaZigZag = 2,  //
    kRLE2 = 3,         //
    kRLE4 = 4          //
  };
  DataHeader();
  DataHeader(uint min, uint max, uint resolution_bits);
  DataHeader(uint min, uint max);
  std::string name;
  std::string uuid;
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
