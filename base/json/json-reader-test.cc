#define CATCH_CONFIG_PREFIX_ALL
#include "json-reader.h"

#include <base/type-trait.h>

#include <algorithm>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <charconv>
#include <iostream>
#include <string>

#include "json-tests.h"
#include "json-writer.h"
#include "pretty-formatter.h"

namespace base {
namespace json {

template <typename T>
std::string_view Signature() {
  return __PRETTY_FUNCTION__;
}

struct Foo {
  bool m0;
  short m1;
  double m2;
  std::array<int, 3> m3;
  std::array<char, 3> m4;
  std::vector<float> m5;
  std::string m6;
  std::map<std::string, int> m7;
  std::map<int, int> m8;
  std::set<std::string> m9;
  std::shared_ptr<int> m10;
  std::unique_ptr<float> m11;
  std::optional<std::string> m12;
  std::tuple<char, int, std::string> m13;

  REFLECTION(Foo, &Foo::m0, &Foo::m1, &Foo::m2, &Foo::m3, &Foo::m4, &Foo::m5,
             &Foo::m6, &Foo::m7, &Foo::m8, &Foo::m9, &Foo::m10, &Foo::m11,
             &Foo::m12, &Foo::m13);
};

struct Bar {
  std::array<char, 3> m1;
};

CATCH_TEST_CASE("basic", "[reflection]") {
  std::string json_str;
  {
    Foo foo;
    foo.m0 = true;
    foo.m1 = 123;
    foo.m2 = 3.14;
    foo.m3 = std::array<int, 3>{1, 2, 3};
    foo.m4 = std::array<char, 3>{'a', 'b', 'c'};
    foo.m5 = std::vector<float>{1.1f, 2.2f, 3.3f};
    foo.m6 = "hello";
    foo.m7 = {{"1", 1}, {"2", 2}, {"3", 3}};
    foo.m8 = {{1, 1}, {2, 2}, {3, 3}};
    foo.m9 = {"123", "456", "789"};
    foo.m10 = std::make_shared<int>(666);
    foo.m11 = std::make_unique<float>(8.88f);
    foo.m12 = std::nullopt;
    foo.m13 = std::make_tuple('c', 777, "world");

    PrettyJsonFormatter formatter;
    ToJson(formatter, foo);
    json_str = std::move(formatter.stream());
  }

  {
    Foo foo;
    std::cout << "json: " << json_str << std::endl;
    if (auto e = FromJson(json_str, foo); e) {
      std::cout << "err: " << e.GetMessage() << std::endl;
    } else {
      PrettyJsonFormatter formatter;
      ToJson(formatter, foo);
      std::cout << formatter.stream() << std::endl;
    }
  }

  {
    Value v;
    std::cout << "json value" << std::endl;
    if (auto e = FromJson(json_str, v); e) {
      std::cout << "err: " << e.GetMessage() << std::endl;
    } else {
      std::cout << v.ToJson(true) << std::endl;
    }
  }

  {
    Value v;
    std::cout << "tmall json" << std::endl;
    if (auto e = FromJson(kTmallJson, v); e) {
      std::cout << "err: " << e.GetMessage() << std::endl;
    } else {
      std::cout << v.ToJson(true) << std::endl;
    }
  }

  {
    std::string_view nums = "2";
    int num = 0;
    std::from_chars(nums.data(), nums.data() + nums.size(), num);
    std::cout << num << std::endl;

    std::array<char, 3> arr;
    std::cout << IsCharArray<decltype(arr)> << std::endl;
    std::cout << IsNonCharArray<decltype(arr)> << std::endl;

    Bar bar;
    std::cout << IsCharArray<decltype(bar.m1)> << std::endl;
  }
}

}  // namespace json
}  // namespace base

int main(int argc, char* argv[]) { return Catch::Session().run(argc, argv); }
