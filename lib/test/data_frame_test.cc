#include "data_frame.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <chrono>

using namespace sensor_compress;

TEST(DataFrameTest, compressEmpty) {
  DataFrame frame;
  frame.Record(FromMs(0), 1);
  frame.Record(FromMs(1), 2);
  frame.Record(FromMs(2), 2);
  frame.Record(FromMs(3), 5);
  frame.Record(FromMs(4), 10);
  std::vector<uint64_t> ms;
  for (auto& t : frame.Times()) ms.push_back(ToMs(t));
  EXPECT_THAT(frame.Values(), ::testing::ElementsAre(1, 2, 2, 5, 10));

  DataHeader reference(0, 4095);
  float value_compression_ratio;
  auto compressed = *frame.Compress(reference, &value_compression_ratio);
  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 2, 5, 10));
  std::vector<uint64_t> ms2;
  for (auto& t : frame2.Times()) ms2.push_back(ToMs(t));
  EXPECT_THAT(ms2, ::testing::ElementsAreArray(ms));
  EXPECT_FLOAT_EQ(value_compression_ratio, 0.5f);
}

TEST(DataFrameTest, compressZigZag) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 1);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 4);

  DataHeader reference(0, 4095);
  reference.value_compressions.push_back(DataHeader::CompressionType::kZigZag);
  float value_compression_ratio;
  auto compressed = *frame.Compress(reference, &value_compression_ratio);

  std::array<uint64_t, 3> compressed64 = {2, 4, 8};
  std::array<uint8_t, 3 * sizeof(uint64_t)> compressed8;
  std::memcpy(compressed8.data(), compressed64.data(), compressed8.size());
  EXPECT_THAT(compressed.values, ::testing::ElementsAreArray(compressed8));

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 4));
  EXPECT_FLOAT_EQ(value_compression_ratio, 0.5f);
}

TEST(DataFrameTest, compressSimple8b) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 1);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 4);
  frame.Record(std::chrono::steady_clock::now(), 8);

  DataHeader reference(0, 4095);
  reference.value_compressions.push_back(DataHeader::CompressionType::kSimple8b);

  float value_compression_ratio;
  auto compressed = *frame.Compress(reference, &value_compression_ratio);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 4, 8));
  EXPECT_FLOAT_EQ(value_compression_ratio, 2.f);
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

  DataHeader reference(0, 4095);
  reference.value_compressions.push_back(DataHeader::CompressionType::kSimple8b);
  float value_compression_ratio, value_compression_ratio_inc;
  auto compressed = *frame.Compress(reference, &value_compression_ratio);
  auto compressed_inc = *frame_inc.Compress(reference, &value_compression_ratio_inc);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  auto frame2_inc = *DataFrame::Decompress(reference, compressed_inc);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAreArray(values));
  EXPECT_FLOAT_EQ(value_compression_ratio, 25.f);
  EXPECT_THAT(frame2_inc.Values(), ::testing::ElementsAreArray(values_inc));
  EXPECT_FLOAT_EQ(value_compression_ratio_inc, 4.f);
}

TEST(DataFrameTest, compressDelta) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 1);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 3);

  DataHeader reference(0, 4095);
  reference.value_compressions.push_back(DataHeader::CompressionType::kDeltaZigZag);
  float value_compression_ratio;
  auto compressed = *frame.Compress(reference, &value_compression_ratio);

  std::array<uint64_t, 3> compressed64 = {0, 2, 2};
  std::array<uint8_t, 3 * sizeof(uint64_t)> compressed8;
  std::memcpy(compressed8.data(), compressed64.data(), compressed8.size());
  EXPECT_THAT(compressed.values, ::testing::ElementsAreArray(compressed8));

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 3));
  EXPECT_FLOAT_EQ(value_compression_ratio, .5f);
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

  DataHeader reference(0, 4095);
  reference.value_compressions.push_back(DataHeader::CompressionType::kDeltaZigZag);
  reference.value_compressions.push_back(DataHeader::CompressionType::kDeltaZigZag);
  reference.value_compressions.push_back(DataHeader::CompressionType::kSimple8b);
  float value_compression_ratio, value_compression_ratio_inc;
  auto compressed = *frame.Compress(reference, &value_compression_ratio);
  auto compressed_inc = *frame_inc.Compress(reference, &value_compression_ratio_inc);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  auto frame2_inc = *DataFrame::Decompress(reference, compressed_inc);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAreArray(values));
  EXPECT_EQ(std::round(value_compression_ratio), 100.f);
  EXPECT_THAT(frame2_inc.Values(), ::testing::ElementsAreArray(values_inc));
  EXPECT_EQ(std::round(value_compression_ratio_inc), 50.f);
}

TEST(DataFrameTest, compressRle) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 1);
  frame.Record(std::chrono::steady_clock::now(), 2);
  frame.Record(std::chrono::steady_clock::now(), 4);
  frame.Record(std::chrono::steady_clock::now(), 8);

  DataHeader reference(0, 4095);
  reference.value_compressions.push_back(DataHeader::CompressionType::kRLE2);
  auto compressed = *frame.Compress(reference);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(1, 2, 4, 8));
}

TEST(DataFrameTest, compressRle2) {
  DataFrame frame;
  frame.Record(std::chrono::steady_clock::now(), 3);
  frame.Record(std::chrono::steady_clock::now(), 3);
  frame.Record(std::chrono::steady_clock::now(), 3);
  frame.Record(std::chrono::steady_clock::now(), 3);

  DataHeader reference(0, 4095);
  reference.value_compressions.push_back(DataHeader::CompressionType::kRLE2);
  auto compressed = *frame.Compress(reference);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAre(3, 3, 3, 3));
}

TEST(DataFrameTest, compressExtendedDeltaRleSimple8b) {
  std::vector<uint> values, values_inc;
  std::vector<steady_time_point_t> times;
  for (int i = 0; i < 200; i++) {
    values.push_back(1);
    values_inc.push_back(i);
    times.push_back(FromMs(i));
  }
  DataFrame frame, frame_inc;
  for (int i = 0; i < values.size(); i++) {
    frame.Record(times[i], values[i]);
    frame_inc.Record(times[i], values_inc[i]);
  }

  DataHeader reference(0, 4095);
  reference.time_compressions.push_back(DataHeader::CompressionType::kDeltaZigZag);
  reference.time_compressions.push_back(DataHeader::CompressionType::kRLE2);
  reference.time_compressions.push_back(DataHeader::CompressionType::kSimple8b);
  reference.value_compressions.push_back(DataHeader::CompressionType::kDeltaZigZag);
  reference.value_compressions.push_back(DataHeader::CompressionType::kRLE2);
  reference.value_compressions.push_back(DataHeader::CompressionType::kSimple8b);
  float value_compression_ratio, time_compression_ratio;
  auto compressed = *frame.Compress(reference, &value_compression_ratio, &time_compression_ratio);
  float value_compression_ratio_inc, time_compression_ratio_inc;
  auto compressed_inc =
      *frame_inc.Compress(reference, &value_compression_ratio_inc, &time_compression_ratio_inc);

  auto frame2 = *DataFrame::Decompress(reference, compressed);
  auto frame2_inc = *DataFrame::Decompress(reference, compressed_inc);
  EXPECT_THAT(frame2.Values(), ::testing::ElementsAreArray(values));
  EXPECT_THAT(frame2.Times(), ::testing::ElementsAreArray(times));
  EXPECT_EQ(std::round(value_compression_ratio), 100.f);
  EXPECT_EQ(std::round(time_compression_ratio), 200.f);
  EXPECT_THAT(frame2_inc.Values(), ::testing::ElementsAreArray(values_inc));
  EXPECT_THAT(frame2_inc.Times(), ::testing::ElementsAreArray(times));
  EXPECT_EQ(std::round(value_compression_ratio_inc), 100.f);
  EXPECT_EQ(std::round(time_compression_ratio_inc), 200.f);
}