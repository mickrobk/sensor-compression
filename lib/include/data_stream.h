#pragma once

#include <chrono>
#include <functional>
#include <tl/expected.hpp>
#include <vector>

#include "data_frame.h"
#include "data_header.h"

namespace sensor_compress {

class DataStream {
 public:
  std::optional<tl::expected<CompressedDataFrame, std::string>> Record(const DataHeader& header,
                                                                       DataFrameValue value);
  std::optional<tl::expected<CompressedDataFrame, std::string>> Record(const DataHeader& header,
                                                                       uint value);

 private:
  DataFrame current_frame_;
};

}  // namespace sensor_compress
