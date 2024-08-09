#pragma once

#include <chrono>
#include <functional>
#include <vector>

#include "data_frame.h"
#include "data_header.h"

namespace sensor_compress {

class DataStream {
 public:
  DataStream(std::function<std::string(const CompressedDataFrame&)> compressor = nullptr);

  bool Record(const DataHeader& header, DataFrameValue value);
  bool Record(const DataHeader& header, uint value);

 private:
  DataFrame current_frame_;
  std::vector<CompressedDataFrame> past_frames_;
  std::function<std::string(const CompressedDataFrame&)> compressor_;
  std::vector<std::string> string_compressed_frames_;
};

}  // namespace sensor_compress
