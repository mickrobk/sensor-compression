
#include "data_frame.h"

namespace sensor_compress {

void DataFrame::Record(steady_time_point_t t, uint value) {
  times_.push_back(t);
  values_.push_back(value);
}

const std::vector<uint>& DataFrame::Values() const { return values_; }
const std::vector<steady_time_point_t>& DataFrame::Times() const { return times_; }

tl::expected<CompressedDataFrame, std::string> DataFrame::Compress(const DataHeader& reference,
                                                                   float* value_compression_ratio,
                                                                   float* time_compression_ratio) const {
  CompressedDataFrame result;
  if (values_.empty()) {
    return result;
  }
  CompressionMemory mem(values_.size());
  TLinearTransform<double> pack(reference.min, reference.max, 0,
                                std::pow(2, reference.resolution_bits));
  for (int i = 0; i < values_.size(); i++) {
    mem.ua[i] = static_cast<uint64_t>(std::round(pack(values_[i])));
  }
  for (auto& c : reference.value_compressions) {
    std::optional<uint64_t> side_channel;
    if (!Compress(c, mem, side_channel)) return tl::unexpected("Unspecified dataframe compression error");
    if (side_channel) {
      result.side_channel.push_back(*side_channel);
    }
  }
  result.values.resize(mem.ua.size() * sizeof(uint64_t));
  std::memcpy(result.values.data(), mem.ua.data(), mem.size * sizeof(uint64_t));
  if (value_compression_ratio)
    *value_compression_ratio = (values_.size() * sizeof(uint) * 1.0f) / result.values.size();

  mem.resize(times_.size());
  for (int i = 0; i < times_.size(); i++) {
    mem.ua[i] = ToMs(times_[i]);
  }
  for (auto& c : reference.time_compressions) {
    std::optional<uint64_t> side_channel;
    if (!Compress(c, mem, side_channel)) return tl::unexpected("Unspecified dataframe compression error");
    if (side_channel) {
      result.side_channel.push_back(*side_channel);
    }
  }
  result.times.resize(mem.ua.size() * sizeof(uint64_t));
  std::memcpy(result.times.data(), mem.ua.data(), mem.size * sizeof(uint64_t));
  if (time_compression_ratio)
    *time_compression_ratio =
        (times_.size() * sizeof(decltype(times_)::value_type) * 1.0f) / result.times.size();

  return result;
}
std::optional<DataFrame> DataFrame::Decompress(const DataHeader& reference,
                                               const CompressedDataFrame& data) {
  DataFrame result;
  CompressionMemory mem(data.values.size() / sizeof(uint64_t));
  auto side_channel_it = data.side_channel.rbegin();
  std::optional<uint64_t> side_channel;
  auto maybe_update_side_channel = [&]() {
    if (side_channel.has_value() || side_channel_it == data.side_channel.rend()) return;
    side_channel = *side_channel_it;
    side_channel_it++;
  };
  if (data.times.empty()) return result;
  mem.resize(data.times.size() / sizeof(uint64_t));
  std::memcpy(mem.ua.data(), data.times.data(), data.times.size());
  for (auto it = reference.time_compressions.rbegin(); it != reference.time_compressions.rend();
       ++it) {
    maybe_update_side_channel();
    if (!Decompress(*it, mem, side_channel)) return std::nullopt;
  }
  for (auto& time : mem.ua) {
    result.times_.push_back(SteadyFromMs(time));
  }

  ///////

  if (data.values.empty()) {
    return result;
  }
  mem.resize(data.values.size() / sizeof(uint64_t));
  std::memcpy(mem.ua.data(), data.values.data(), data.values.size());
  for (auto it = reference.value_compressions.rbegin(); it != reference.value_compressions.rend();
       ++it) {
    maybe_update_side_channel();
    if (!Decompress(*it, mem, side_channel)) return std::nullopt;
  }
  TLinearTransform<double> unpack(0, std::pow(2, reference.resolution_bits), reference.min,
                                  reference.max);
  for (auto& value : mem.ua) {
    result.values_.push_back(static_cast<uint>(std::round(unpack(value))));
  }
  return result;
}

bool DataFrame::Decompress(DataHeader::CompressionType compression_type, CompressionMemory& mem,
                           std::optional<uint64_t>& side_channel) {
  switch (compression_type) {
    case DataHeader::CompressionType::kSimple8b: {
      if (!side_channel) {
        printf("kSimple8b missing side channel\n");
        return false;
      }
      auto uncompressed_size = *side_channel;
      side_channel = std::nullopt;
      mem.resize(uncompressed_size);
      size_t out_len = Simple8bDecode(mem.ua.data(), uncompressed_size, mem.ub.data());
      std::memcpy(mem.ua.data(), mem.ub.data(), out_len * sizeof(uint64_t));
      mem.resize(out_len);
      return true;
    }
    case DataHeader::CompressionType::kZigZag:
      ZigZagDecode(mem.ua.data(), mem.size);
      return true;
    case DataHeader::CompressionType::kDeltaZigZag: {
      if (!side_channel) {
        printf("DeltaZigZag missing side channel\n");
        return false;
      }
      auto& initial_value = *side_channel;
      side_channel = std::nullopt;
      for (int i = 0; i < mem.size; i++) {
        mem.sa[i] = static_cast<int32_t>(mem.ua[i]);
      }
      ZigZagDecode(mem.sa.data(), mem.size);
      if (!DeltaDecode(mem.sa.data(), mem.size, initial_value, mem.ua.data())) return false;
      return true;
    }
    case DataHeader::CompressionType::kRLE2:
    case DataHeader::CompressionType::kRLE4: {
      if (!side_channel) {
        printf("RLE missing side channel\n");
        return false;
      }
      auto& uncompressed_size = *side_channel;
      auto compressed_size = mem.ua.size();
      side_channel = std::nullopt;
      mem.resize(uncompressed_size);
      if (!RleDecode(mem.ua.data(), compressed_size, mem.ub.data(), uncompressed_size)) {
        printf("RLEz decode failed\n");
        return false;
      }
      std::memcpy(mem.ua.data(), mem.ub.data(), mem.ub.size() * sizeof(uint64_t));
      return true;
    }
  }
  return false;
}
bool DataFrame::Compress(DataHeader::CompressionType compression_type, CompressionMemory& mem,
                         std::optional<uint64_t>& side_channel) const {
  switch (compression_type) {
    case DataHeader::CompressionType::kSimple8b: {
      side_channel = mem.ua.size();
      size_t out_len = Simple8bEncode(mem.ua.data(), mem.size, mem.ub.data());
      std::memcpy(mem.ua.data(), mem.ub.data(), out_len * sizeof(uint64_t));
      mem.resize(out_len);
      return true;
    }
    case DataHeader::CompressionType::kZigZag:
      ZigZagEncode(mem.ua.data(), mem.size);
      return true;
    case DataHeader::CompressionType::kDeltaZigZag: {
      side_channel = 0;
      if (!DeltaEncode(mem.ua.data(), mem.ua.size(), &*side_channel, mem.sa.data())) return false;
      ZigZagEncode(mem.sa.data(), mem.size);
      for (int i = 0; i < mem.size; i++) {
        mem.ua[i] = static_cast<uint64_t>(mem.sa[i]);
      }
      return true;
    }
    case DataHeader::CompressionType::kRLE2:
    case DataHeader::CompressionType::kRLE4: {
      size_t min_run = compression_type == DataHeader::CompressionType::kRLE2 ? 2 : 4;
      side_channel = mem.ua.size();
      auto out_len = RleEncode(min_run, mem.ua.data(), mem.size, mem.ub.data());
      if (!out_len) {
        printf("RLE encode failed\n");
        return false;
      }
      mem.resize(*out_len);
      std::memcpy(mem.ua.data(), mem.ub.data(), *out_len * sizeof(uint64_t));
      return true;
    }
  }
  return false;
}

}  // namespace sensor_compress
