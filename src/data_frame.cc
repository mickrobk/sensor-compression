
#include "data_frame.h"

namespace sensor_compress {

void DataFrame::Record(steady_time_point_t, uint value) {
  times_.push_back(std::chrono::steady_clock::now());
  values_.push_back(value);
}

const std::vector<uint>& DataFrame::Values() const { return values_; }

std::optional<CompressedDataFrame> DataFrame::Compress(const DataFrameReference& reference) const {
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
    std::optional<CompressionSideChannel> side_channel;
    if (!Compress(c, mem, side_channel)) return std::nullopt;
    if (side_channel) {
      result.side_channel.push_back(*side_channel);
    }
  }
  result.values.resize(mem.ua.size() * sizeof(uint64_t));
  std::memcpy(result.values.data(), mem.ua.data(), mem.size * sizeof(uint64_t));
  result.value_compression_ratio =
      (values_.size() * sizeof(decltype(values_)::value_type) * 1.0f) / result.values.size();

  return result;
}
std::optional<DataFrame> DataFrame::Decompress(const DataFrameReference& reference,
                                               const CompressedDataFrame& data) {
  DataFrame result;
  CompressionMemory mem(data.values.size() / sizeof(uint64_t));
  if (data.values.empty()) {
    return result;
  }
  std::memcpy(mem.ua.data(), data.values.data(), data.values.size());
  auto side_channel_it = data.side_channel.rbegin();
  std::optional<CompressionSideChannel> side_channel;
  auto maybe_update_side_channel = [&]() {
    if (side_channel.has_value() || side_channel_it == data.side_channel.rend()) return;
    side_channel = *side_channel_it;
    side_channel_it++;
  };
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

bool DataFrame::Decompress(DataFrameReference::CompressionType compression_type,
                           CompressionMemory& mem,
                           std::optional<CompressionSideChannel>& side_channel) {
  switch (compression_type) {
    case DataFrameReference::CompressionType::kSimple8b: {
      if (!side_channel) {
        printf("kSimple8b missing side channel\n");
        return false;
      }
      auto uncompressed_size = side_channel->simple8b_uncompressed_size;
      side_channel = std::nullopt;
      mem.resize(uncompressed_size);
      size_t out_len = Simple8bDecode(mem.ua.data(), uncompressed_size, mem.ub.data());
      std::memcpy(mem.ua.data(), mem.ub.data(), out_len * sizeof(uint64_t));
      mem.resize(out_len);
      return true;
    }
    case DataFrameReference::CompressionType::kZigZag:
      ZigZagDecode(mem.ua.data(), mem.size);
      return true;
    case DataFrameReference::CompressionType::kDeltaZigZag: {
      if (!side_channel) {
        printf("DeltaZigZag missing side channel\n");
        return false;
      }
      auto initial_value = side_channel->initial_value;
      side_channel = std::nullopt;
      for (int i = 0; i < mem.size; i++) {
        mem.sa[i] = static_cast<int32_t>(mem.ua[i]);
      }
      ZigZagDecode(mem.sa.data(), mem.size);
      if (!DeltaDecode(mem.sa.data(), mem.size, initial_value, mem.ua.data())) return false;
      return true;
    }
  }
}
bool DataFrame::Compress(DataFrameReference::CompressionType compression_type,
                         CompressionMemory& mem,
                         std::optional<CompressionSideChannel>& side_channel) const {
  switch (compression_type) {
    case DataFrameReference::CompressionType::kSimple8b: {
      side_channel = CompressionSideChannel();
      side_channel->simple8b_uncompressed_size = mem.ua.size();
      size_t out_len = Simple8bEncode(mem.ua.data(), mem.size, mem.ub.data());
      std::memcpy(mem.ua.data(), mem.ub.data(), out_len * sizeof(uint64_t));
      mem.resize(out_len);
      return true;
    }
    case DataFrameReference::CompressionType::kZigZag:
      ZigZagEncode(mem.ua.data(), mem.size);
      return true;
    case DataFrameReference::CompressionType::kDeltaZigZag: {
      side_channel = CompressionSideChannel();
      if (!DeltaEncode(mem.ua.data(), mem.ua.size(), &side_channel->initial_value, mem.sa.data()))
        return false;
      ZigZagEncode(mem.sa.data(), mem.size);
      for (int i = 0; i < mem.size; i++) {
        mem.ua[i] = static_cast<uint64_t>(mem.sa[i]);
      }
      return true;
    }
  }
}
uint64_t DataFrame::AsMs(steady_time_point_t tp) const {
  auto mills = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  return static_cast<uint64_t>(mills.count());
}

}  // namespace sensor_compress