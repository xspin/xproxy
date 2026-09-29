#include <gtest/gtest.h>
#include <string>

#include "result.hpp"

TEST(result, Result) {
    result::Result<int, std::string> r =
        result::Result<int, std::string>::ok(123);
    EXPECT_TRUE(r.is_ok());
    EXPECT_FALSE(r.is_err());
    EXPECT_EQ(r.unwrap(), 123);
    r = result::Result<int, std::string>::err("errormsg");
    EXPECT_FALSE(r.is_ok());
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(r.unwrap_err(), "errormsg");
}

TEST(result, Res) {
    result::Res r = result::Ok();
    EXPECT_TRUE(r.is_ok());
    EXPECT_FALSE(r.is_err());
    EXPECT_EQ(r.unwrap(), true);

    r = result::Errf("errmsg %d", 1123);
    EXPECT_FALSE(r.is_ok());
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(r.unwrap_err(), "errmsg 1123");

    r = result::Errf("abcd");
    EXPECT_EQ(r.unwrap_err(), "abcd");
}
