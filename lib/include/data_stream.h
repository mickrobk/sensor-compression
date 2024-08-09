#pragma once

#include <chrono>
#include <functional>
#include <vector>

#include "data_frame.h"
#include "data_header.h"

namespace sensor_compress {

class DataStream {
 public:
  std::optional<CompressedDataFrame> Record(const DataHeader& header, DataFrameValue value);
  std::optional<CompressedDataFrame> Record(const DataHeader& header, uint value);

 private:
  DataFrame current_frame_;
};

}  // namespace sensor_compress
