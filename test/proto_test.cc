#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pb_decode.h"
#include "pb_encode.h"
#include "proto/sensor.pb.h"
#include "simple8b.h"

TEST(ProtoTest, compiles) {
  // uint8_t buffer[256];
  // pb_ostream_t ostream;
  // pb_istream_t istream;
  // size_t written;

  // TestMessageWithOptions original = TestMessageWithOptions_init_zero;
  // strcpy(original.str, "Hello");

  // TestMessageWithOptions original = TestMessageWithOptions_init_zero;
  // strcpy(original.str,"Hello");

  // ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  // TEST(pb_encode(&ostream, &TestMessageWithOptions_msg, &original));

  // written = ostream.bytes_written;

  // istream = pb_istream_from_buffer(buffer, written);

  // TestMessageWithOptions decoded = TestMessageWithOptions_init_zero;

  // TEST(pb_decode(&istream, &TestMessageWithOptions_msg, &decoded));

  // TEST(strcmp(decoded.str,"Hello") == 0);
}