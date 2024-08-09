#include "data_header.h"
#include "data_stream.h"
#include "gtest/gtest.h"

namespace sensor_compress {

class DataStreamTest : public ::testing::Test {
 protected:
  DataStream data_stream;
  DataHeader header{0, 10000};
};

TEST_F(DataStreamTest, RecordWithValue) {
  auto result = data_stream.Record(header, 42);
  EXPECT_FALSE(result.has_value());
}

TEST_F(DataStreamTest, RecordWithDataFrameValue) {
  DataFrameValue value{std::chrono::steady_clock::now(), 42};
  auto result = data_stream.Record(header, value);
  EXPECT_FALSE(result.has_value());
}

TEST_F(DataStreamTest, RecordWithFrameSizeLimit) {
  header.frame_size = 1;
  auto result1 = data_stream.Record(header, 42);
  EXPECT_FALSE(result1.has_value());

  auto result2 = data_stream.Record(header, 43);
  EXPECT_TRUE(result2.has_value());
}

}  // namespace sensor_compress
