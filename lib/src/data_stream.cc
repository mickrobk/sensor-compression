#include "data_stream.h"

#include <cstdio>

namespace sensor_compress {

tl::expected<CompressedDataFrame, std::string> DataStream::Record(const DataHeader& header,
                                                                  DataFrameValue value) {
  if (current_frame_.size() >= header.frame_size) {
    auto temp = current_frame_.Compress(header);
    current_frame_.Clear();
    return temp;
  }
  current_frame_.Record(value);
  return tl::unexpected("Frame size not reached");
}

tl::expected<CompressedDataFrame, std::string> DataStream::Record(const DataHeader& header, uint value) {
  return Record(header, {std::chrono::steady_clock::now(), value});
}

}  // namespace sensor_compress
