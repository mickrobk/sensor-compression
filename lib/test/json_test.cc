#include "json.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "data_frame.h"

using namespace sensor_compress;

TEST(JsonTest, dataframe) {
  DataHeader dh1(0, 10000);
  dh1.name = "hi";
  dh1.value_compressions = {DataHeader::CompressionType::kSimple8b,
                            DataHeader::CompressionType::kRLE2};
  auto jdh = Json(dh1);
  EXPECT_EQ(jdh["name"], "hi");
  EXPECT_EQ(jdh["version"], 1);
  EXPECT_THAT(jdh["value_compressions"], testing::ElementsAre(0, 3));

  auto dh2 = jdh.template get<DataHeader>();
  EXPECT_EQ(dh1.name, dh2.name);
  EXPECT_EQ(dh1.min, dh2.min);  // hi!
  EXPECT_EQ(dh1.max, dh2.max);
  EXPECT_EQ(dh1.resolution_bits, dh2.resolution_bits);
}