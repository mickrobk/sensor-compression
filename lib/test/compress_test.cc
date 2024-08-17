#include "compress.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

#include "log_container.h"
#include "time_util.h"

using namespace sensor_compress;

TEST(CompressTest, delta) {
  std::vector<uint64_t> decoded = {5, 2, 3, 2, 12};
  std::vector<int32_t> encoded;
  uint64_t initial = 0;
  encoded.resize(decoded.size());
  ASSERT_TRUE(DeltaEncode(decoded.data(), decoded.size(), &initial, encoded.data()));
  EXPECT_THAT(encoded, ::testing::ElementsAre(0, -3, 1, -1, 10));
  EXPECT_EQ(initial, 5);

  decoded.clear();
  decoded.resize(encoded.size());
  ASSERT_TRUE(DeltaDecode(encoded.data(), encoded.size(), initial, decoded.data()));
  EXPECT_THAT(decoded, ::testing::ElementsAre(5, 2, 3, 2, 12));
}

TEST(CompressTest, encode_delta_oob) {
  std::vector<int32_t> encoded;
  uint64_t initial = 0;
  {
    std::vector<uint64_t> foo = {0llu, std::numeric_limits<int32_t>::max() + 1llu};
    encoded.resize(foo.size());
    ASSERT_FALSE(DeltaEncode(foo.data(), foo.size(), &initial, encoded.data()));
  }
  {
    std::vector<uint64_t> foo = {std::numeric_limits<int32_t>::max(), 0};
    encoded.resize(foo.size());
    ASSERT_FALSE(DeltaEncode(foo.data(), foo.size(), &initial, encoded.data()));
  }
}

TEST(CompressTest, decode_delta_oob) {
  {
    std::vector<int32_t> encoded = {1};
    uint64_t initial = std::numeric_limits<uint64_t>::max();
    std::vector<uint64_t> decoded;
    decoded.resize(encoded.size());
    ASSERT_FALSE(DeltaDecode(encoded.data(), encoded.size(), initial, decoded.data()));
  }
  {
    std::vector<int32_t> encoded = {-1};
    uint64_t initial = 0;
    std::vector<uint64_t> decoded;
    decoded.resize(encoded.size());
    ASSERT_FALSE(DeltaDecode(encoded.data(), encoded.size(), initial, decoded.data()));
  }
}

TEST(CompressTest, zigzag) {
  int64_t foo[] = {0, 1, 2, 4, -2, -1};
  ZigZagEncode(foo, 6);
  EXPECT_THAT(foo, ::testing::ElementsAre(0, 2, 4, 8, 3, 1));
  ZigZagDecode(foo, 6);
  EXPECT_THAT(foo, ::testing::ElementsAre(0, 1, 2, 4, -2, -1));
}

TEST(CompressTest, simple8b) {
  std::vector<uint64_t> foo = {0, 1, 2, 4, 2, 1};
  size_t orginal_len = foo.size();
  std::vector<uint64_t> working;
  working.resize(foo.size());
  auto out_len = Simple8bEncode(&foo[0], foo.size(), &working[0]);
  EXPECT_EQ(out_len, 1);
  working.resize(out_len);
  foo.clear();
  foo.resize(orginal_len);
  auto decoded_len = Simple8bDecode(&working[0], orginal_len, &foo[0]);
  EXPECT_EQ(decoded_len, orginal_len);
  EXPECT_THAT(foo, ::testing::ElementsAre(0, 1, 2, 4, 2, 1));
}

TEST(CompressTest, rleNoRun) {
  std::vector<uint64_t> foo = {0, 1, 2};
  size_t orginal_len = foo.size();
  std::vector<uint64_t> compressed;
  compressed.resize(foo.size());
  auto out_len = RleEncode(2, foo.data(), foo.size(), compressed.data());
  EXPECT_EQ(out_len, 3);
  compressed.resize(*out_len);
  EXPECT_THAT(compressed, ::testing::ElementsAre(0 << 1, 1 << 1, 2 << 1));

  foo.clear();
  foo.resize(orginal_len);
  EXPECT_TRUE(RleDecode(compressed.data(), compressed.size(), foo.data(), orginal_len));
  EXPECT_THAT(foo, ::testing::ElementsAre(0, 1, 2));
}

TEST(CompressTest, rleRun) {
  std::vector<uint64_t> foo = {0, 0, 0, 1, 2, 2};
  size_t orginal_len = foo.size();
  std::vector<uint64_t> compressed;
  compressed.resize(foo.size());
  auto out_len = RleEncode(2, foo.data(), foo.size(), compressed.data());
  EXPECT_EQ(out_len, 5);
  compressed.resize(*out_len);
  EXPECT_THAT(compressed, ::testing::ElementsAre(3 << 1 | 1, 0 << 1 | 0,  //
                                                 1 << 1 | 0,              //
                                                 2 << 1 | 1, 2 << 1 | 0   //
                                                 ));
  foo.clear();
  foo.resize(orginal_len);
  EXPECT_TRUE(RleDecode(compressed.data(), compressed.size(), foo.data(), orginal_len));
  EXPECT_THAT(foo, ::testing::ElementsAre(0, 0, 0, 1, 2, 2));
}

TEST(CompressTest, rleLongRun) {
  std::vector<uint64_t> foo = {0, 1, 2, 2, 3, 4, 4, 4, 4, 45, 6, 7};
  size_t orginal_len = foo.size();
  std::vector<uint64_t> compressed;
  compressed.resize(foo.size());
  auto out_len = RleEncode(3, foo.data(), foo.size(), compressed.data());
  compressed.resize(*out_len);
  foo.clear();
  foo.resize(orginal_len);
  EXPECT_TRUE(RleDecode(compressed.data(), compressed.size(), foo.data(), orginal_len));
  EXPECT_THAT(foo, ::testing::ElementsAre(0, 1, 2, 2, 3, 4, 4, 4, 4, 45, 6, 7));
}

TEST(CompressTest, rleTime) {
  uint64_t a = ToMs(std::chrono::steady_clock::now());
  uint64_t b = ToMs(std::chrono::steady_clock::now());
  uint64_t c = ToMs(std::chrono::steady_clock::now());
  uint64_t d = ToMs(std::chrono::steady_clock::now());
  std::vector<uint64_t> foo = {a, b, c, d};
  size_t orginal_len = foo.size();
  std::vector<uint64_t> compressed;
  compressed.resize(foo.size());
  auto out_len = RleEncode(2, foo.data(), foo.size(), compressed.data());
  compressed.resize(*out_len);
  foo.clear();
  foo.resize(orginal_len);
  EXPECT_TRUE(RleDecode(compressed.data(), compressed.size(), foo.data(), orginal_len));
  EXPECT_THAT(foo, ::testing::ElementsAre(a, b, c, d));
}
TEST(CompressTest, ToFromBytes) {
  std::vector<uint64_t> input = {1, 2, 3, 4, 5};
  auto bytes = to_bytes(input);
  EXPECT_EQ(bytes.size(), input.size() * sizeof(uint64_t));
  
  auto result = from_bytes(bytes);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), input);

  bytes.pop_back();
  result = from_bytes(bytes);
  EXPECT_FALSE(result.has_value());
}
