#define CATCH_CONFIG_PREFIX_ALL
#include "json-writer.h"

#include <base/type-trait.h>

#include <algorithm>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

#include "pretty-formatter.h"

namespace base {
namespace json {

template <typename T>
std::string_view Signature() {
  return __PRETTY_FUNCTION__;
}

struct Foo {
  int m1;
  double m2;
  std::array<char, 8> m3;
  std::string m4;
  std::vector<int> m5;
  std::map<int, std::string_view> m6;
  std::optional<std::string> m7;
  std::unique_ptr<int> m8;
  std::variant<char, int, double> m9;
  std::tuple<float, char> m10;

  REFLECTION(Foo, &Foo::m1, &Foo::m2, &Foo::m3, &Foo::m4, &Foo::m5, &Foo::m6,
             &Foo::m7, &Foo::m8, &Foo::m9, &Foo::m10);
};

CATCH_TEST_CASE("basic", "[reflection]") {
  {
    Foo foo;
    foo.m1 = 123;
    foo.m2 = 3.14;
    foo.m3 = {'a', 'b', 'c', 'd', '\0'};
    foo.m4 = "hello";
    foo.m5 = {1, 2, 3, 4};
    foo.m6 = {{1, "1"}, {2, "2"}, {3, "3"}};
    foo.m7 = "world";
    foo.m8 = std::make_unique<int>(666);
    foo.m9 = 777;
    foo.m10 = std::make_tuple(1.11f, 'c');

    PrettyJsonFormatter formatter;
    ToJson(formatter, foo);
    std::cout << formatter.stream() << std::endl;
  }
}

}  // namespace json
}  // namespace base

int main(int argc, char* argv[]) { return Catch::Session().run(argc, argv); }
