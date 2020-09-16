#include "iobuffer.h"
#include <gtest/gtest.h>
#include <string>

using epoll_threadpool::IOBuffer;
using std::string;

TEST(IOBufferTest, AppendBytes) {
  IOBuffer buf;
  const char *a = "abc", *b = "def";

  buf.append(a, 3);
  buf.append(b, 3);
  ASSERT_EQ(6, buf.size());
  ASSERT_EQ(NULL, buf.pulldown(7));
  ASSERT_EQ(string("abcdef"), string(buf.pulldown(6), 6));
  ASSERT_EQ(true, buf.consume(1));
  ASSERT_EQ(5, buf.size());
  ASSERT_EQ(NULL, buf.pulldown(6));
  ASSERT_EQ(string("bcdef"), string(buf.pulldown(5), 5));
  ASSERT_EQ(true, buf.consume(3));
  ASSERT_EQ(2, buf.size());
  ASSERT_EQ(string("ef"), string(buf.pulldown(2), 2));
  ASSERT_EQ(NULL, buf.pulldown(3));
}

TEST(IOBufferTest, ConsumeBeforePullDown) {
  IOBuffer buf;
  const char *a = "abc", *b = "def";

  buf.append(a, 3);
  buf.append(b, 3);
  ASSERT_EQ(6, buf.size());
  ASSERT_EQ(true, buf.consume(1));
  ASSERT_EQ(5, buf.size());
  ASSERT_EQ(NULL, buf.pulldown(6));
  ASSERT_EQ(string("bcdef"), string(buf.pulldown(5), 5));
  ASSERT_EQ(true, buf.consume(5));
  ASSERT_EQ(0, buf.size());
  ASSERT_EQ(NULL, buf.pulldown(1));
  ASSERT_EQ(NULL, buf.pulldown(0));
}
