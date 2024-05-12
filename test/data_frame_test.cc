#include "data_frame.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <chrono>

using namespace sensor_compress;

TEST(DataFrameTest, compressEmpty) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 1);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 5);
  frame.Record(std::chrono::steady_clock::now(), 10);
  EXPECT_THAT(frame.Values(), ::testing::ElementsAre(1, 2, 2, 5, 10));

  DataFrameReference reference(0, 4096, 12);
  auto compressed = *frame.Compress(reference);
  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 2, 5, 10));
  EXPECT_FLOAT_EQ(compressed.value_compression_ratio, 0.5f);
}

TEST(DataFrameTest, compressZigZag) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 1);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 4);

  DataFrameReference reference(0, 4096, 12);
  reference.value_compressions.push_back(DataFrameReference::CompressionType::kZigZag);
  auto compressed = *frame.Compress(reference);

  std::array<uint64_t, 3> compressed64 = {2, 4, 8};
  std::array<uint8_t, 3 * sizeof(uint64_t)> compressed8;
  std::memcpy(compressed8.data(), compressed64.data(), compressed8.size());
  EXPECT_THAT(compressed.values, ::testing::ElementsAreArray(compressed8));

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 4));
  EXPECT_FLOAT_EQ(compressed.value_compression_ratio, 0.5f);
}

TEST(DataFrameTest, compressSimple8b) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 1);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 4);
  frame.Record(std::chrono::steady_clock::now(), 8);

  DataFrameReference reference(0, 4096, 12);
  reference.value_compressions.push_back(DataFrameReference::CompressionType::kSimple8b);
  auto compressed = *frame.Compress(reference);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 4, 8));
  EXPECT_FLOAT_EQ(compressed.value_compression_ratio, 2.f);
}

TEST(DataFrameTest, compressExtendedSimple8b) {
  std::vector<uint> values, values_inc;
  for (int i = 0; i < 200; i++) {
    values.push_back(1);
    values_inc.push_back(i);
  }
  DataFrame frame, frame_inc;
  for (int i = 0; i < values.size(); i++) {
    frame.Record(std::chrono::steady_clock::now(), values[i]);
    frame_inc.Record(std::chrono::steady_clock::now(), values_inc[i]);
  }

  DataFrameReference reference(0, 4096, 12);
  reference.value_compressions.push_back(DataFrameReference::CompressionType::kSimple8b);
  auto compressed = *frame.Compress(reference);
  auto compressed_inc = *frame_inc.Compress(reference);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  auto frame2_inc = *DataFrame::Decompress(reference, compressed_inc);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAreArray(values));
  EXPECT_FLOAT_EQ(compressed.value_compression_ratio, 25.f);
  EXPECT_THAT(frame2_inc.Values(), ::testing::ElementsAreArray(values_inc));
  EXPECT_FLOAT_EQ(compressed_inc.value_compression_ratio, 4.f);
}

TEST(DataFrameTest, compressDelta) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 1);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 3);

  DataFrameReference reference(0, 4096, 12);
  reference.value_compressions.push_back(DataFrameReference::CompressionType::kDeltaZigZag);
  auto compressed = *frame.Compress(reference);

  std::array<uint64_t, 3> compressed64 = {0, 2, 2};
  std::array<uint8_t, 3 * sizeof(uint64_t)> compressed8;
  std::memcpy(compressed8.data(), compressed64.data(), compressed8.size());
  EXPECT_THAT(compressed.values, ::testing::ElementsAreArray(compressed8));

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 3));
  EXPECT_FLOAT_EQ(compressed.value_compression_ratio, .5f);
}

TEST(DataFrameTest, compressExtendedDeltaSimple8b) {
  std::vector<uint> values, values_inc;
  for (int i = 0; i < 200; i++) {
    values.push_back(1);
    values_inc.push_back(i);
  }
  DataFrame frame, frame_inc;
  for (int i = 0; i < values.size(); i++) {
    frame.Record(std::chrono::steady_clock::now(), values[i]);
    frame_inc.Record(std::chrono::steady_clock::now(), values_inc[i]);
  }

  DataFrameReference reference(0, 4096, 12);
  reference.value_compressions.push_back(DataFrameReference::CompressionType::kDeltaZigZag);
  reference.value_compressions.push_back(DataFrameReference::CompressionType::kDeltaZigZag);
  reference.value_compressions.push_back(DataFrameReference::CompressionType::kSimple8b);
  auto compressed = *frame.Compress(reference);
  auto compressed_inc = *frame_inc.Compress(reference);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  auto frame2_inc = *DataFrame::Decompress(reference, compressed_inc);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAreArray(values));
  EXPECT_EQ(std::round(compressed.value_compression_ratio), 100.f);
  EXPECT_THAT(frame2_inc.Values(), ::testing::ElementsAreArray(values_inc));
  EXPECT_EQ(std::round(compressed_inc.value_compression_ratio), 50.f);
}