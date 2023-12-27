#define CATCH_CONFIG_PREFIX_ALL
#include "strings.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>

namespace base {

CATCH_TEST_CASE("basic", "[strings]") {
  {
    CATCH_REQUIRE(StartsWith("hi, this is a string", "hi"));
    CATCH_REQUIRE(!StartsWith("hi, this is a string", "hi "));

    CATCH_REQUIRE(EndsWith("hi, this is a string", "string"));
    CATCH_REQUIRE(!EndsWith("hi, this is a string", "string "));
  }

  {
    std::string str("aBcD/:eF ");
    auto res = ToLower(str);
    CATCH_REQUIRE(res == "abcd/:ef ");
  }

  {
    std::string str("aBcD/:eF ");
    auto res = ToLowerInplace(str);
    CATCH_REQUIRE(res == "abcd/:ef ");
    CATCH_REQUIRE(res == str);
  }

  {
    std::string str("   abc   ");
    CATCH_REQUIRE(Trim(str) == "abc");
    CATCH_REQUIRE(RTrim(str) == "   abc");
    CATCH_REQUIRE(LTrim(str) == "abc   ");
  }

  {
    std::string str("   123");
    CATCH_REQUIRE(Trim(str) == "123");
    CATCH_REQUIRE(RTrim(str) == "   123");
    CATCH_REQUIRE(LTrim(str) == "123");
  }

  {
    std::string str("xyz   ");
    CATCH_REQUIRE(Trim(str) == "xyz");
    CATCH_REQUIRE(RTrim(str) == "xyz");
    CATCH_REQUIRE(LTrim(str) == "xyz   ");
  }

  {
    std::string str;
    CATCH_REQUIRE(Trim(str) == "");
    CATCH_REQUIRE(LTrim(str) == "");
    CATCH_REQUIRE(RTrim(str) == "");
  }

  {
    std::string str("   ");
    CATCH_REQUIRE(Trim(str) == "");
    CATCH_REQUIRE(LTrim(str) == "");
    CATCH_REQUIRE(RTrim(str) == "");
  }

  {
    std::string str("aaa:123,bbb:456,ccc:789");
    auto res = Tokenize(str, ",");

    CATCH_REQUIRE(res.size() == 3);
    CATCH_REQUIRE(res[0] == "aaa:123");
    CATCH_REQUIRE(res[1] == "bbb:456");
    CATCH_REQUIRE(res[2] == "ccc:789");

    {
      auto sub_res = Tokenize(res[0], ":");
      CATCH_REQUIRE(sub_res.size() == 2);
      CATCH_REQUIRE(sub_res[0] == "aaa");
      CATCH_REQUIRE(sub_res[1] == "123");
    }

    {
      auto sub_res = Tokenize(res[1], ":");
      CATCH_REQUIRE(sub_res.size() == 2);
      CATCH_REQUIRE(sub_res[0] == "bbb");
      CATCH_REQUIRE(sub_res[1] == "456");
    }

    {
      auto sub_res = Tokenize(res[2], ":");
      CATCH_REQUIRE(sub_res.size() == 2);
      CATCH_REQUIRE(sub_res[0] == "ccc");
      CATCH_REQUIRE(sub_res[1] == "789");
    }
  }

  {
    std::string str(",,,");
    auto res = Tokenize(str, ",", false);

    CATCH_REQUIRE(res.size() == 4);
    CATCH_REQUIRE(res[0] == "");
    CATCH_REQUIRE(res[1] == "");
    CATCH_REQUIRE(res[2] == "");
    CATCH_REQUIRE(res[3] == "");
  }

  {
    std::string str("1,");
    auto res = Tokenize(str, ",", false);

    CATCH_REQUIRE(res.size() == 2);
    CATCH_REQUIRE(res[0] == "1");
    CATCH_REQUIRE(res[1] == "");
  }

  {
    std::string str("1,");
    auto res = Tokenize(str, ":");

    CATCH_REQUIRE(res.size() == 1);
    CATCH_REQUIRE(res[0] == "1,");
  }

  {
    std::string str("123ABC456ABC789");
    auto res = Tokenize(str, "ABC");

    CATCH_REQUIRE(res.size() == 3);
    CATCH_REQUIRE(res[0] == "123");
    CATCH_REQUIRE(res[1] == "456");
    CATCH_REQUIRE(res[2] == "789");
  }

  {
    std::string str("123  456 789 ");
    auto res = Tokenize(str, " ", true);

    CATCH_REQUIRE(res.size() == 3);
    CATCH_REQUIRE(res[0] == "123");
    CATCH_REQUIRE(res[1] == "456");
    CATCH_REQUIRE(res[2] == "789");
  }
}

}  // namespace base

int main(int argc, char* argv[]) { return Catch::Session().run(argc, argv); }
