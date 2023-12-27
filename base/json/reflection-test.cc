#define CATCH_CONFIG_PREFIX_ALL
#include "reflection.h"

#include <base/type-trait.h>

#include <algorithm>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

namespace base {
namespace json {

struct Foo {
  int ab;
  float cd;
  char ef;

  REFLECTION(Foo, &Foo::ab, &Foo::cd, &Foo::ef);
};

CATCH_TEST_CASE("basic", "[reflection]") {
  {
    CATCH_REQUIRE(ARG_COUNT(a, b, c) == 3);

    CATCH_REQUIRE(IsReflectable<Foo>);

    Foo foo;
    foo.ab = 3;
    foo.cd = 1.23f;
    foo.ef = 'c';

    auto reflect_mems = foo.reflect_members(foo);

    CATCH_REQUIRE(reflect_mems.field_size() == 3);

    auto arr = reflect_mems.field_name();
    std::for_each(arr.begin(), arr.end(),
                  [](auto& elem) { std::cout << elem << std::endl; });


    auto refs = reflect_mems.field_ptr();
    auto& [a, b, c] = refs;

    std::cout << foo.*a << std::endl;

  }
}

}  // namespace json
}  // namespace base

int main(int argc, char* argv[]) { return Catch::Session().run(argc, argv); }
