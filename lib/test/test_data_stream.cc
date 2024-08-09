#include "data_stream.h"
#include "data_header.h"
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

TEST_F(DataStreamTest, RecordWithEmptyHeader) {
  DataHeader empty_header;
  auto result = data_stream.Record(empty_header, 42);
  EXPECT_FALSE(result.has_value());
}

TEST_F(DataStreamTest, RecordWithFrameSizeLimit) {
  header.frame_size = 1;
  auto result1 = data_stream.Record(header, 42);
  EXPECT_FALSE(result1.has_value());

  auto result2 = data_stream.Record(header, 43);
  EXPECT_TRUE(result2.has_value());
  EXPECT_EQ(result2->values.size(), 1);
  EXPECT_EQ(result2->values[0], 42);
}

}  // namespace sensor_compress

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
