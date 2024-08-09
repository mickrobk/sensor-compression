#pragma once

#include <chrono>
#include <vector>

#include "data_frame.h"
#include "data_header.h"

namespace sensor_compress {

class Foo {};

class DataStream {
 public:
  bool Record(const DataHeader& header, DataFrameValue value);
  bool Record(const DataHeader& header, uint value);

 private:
  DataFrame current_frame_;
  std::vector<CompressedDataFrame> past_frames_;
};

}  // namespace sensor_compress
