#include "data_stream.h"

#include <cstdio>

namespace sensor_compress {

DataStream::DataStream(std::function<std::string(const CompressedDataFrame&)> compressor)
    : compressor_(compressor) {}

bool DataStream::Record(const DataHeader& header, DataFrameValue value) {
  if (current_frame_.size() >= header.frame_size) {
    if (auto compressed = current_frame_.Compress(header)) {
      if (compressor_) {
        string_compressed_frames_.push_back(compressor_(*compressed));
      } else {
        raw_past_frames_.push_back(*std::move(compressed));
      }
      current_frame_.Clear();
      return true;
    } else {
      printf("Failed to compress current frame\n");
      return false;
    }
  }
  return false;
}

bool DataStream::Record(const DataHeader& header, uint value) {
  return Record(header, {std::chrono::steady_clock::now(), value});
}

}  // namespace sensor_compress
