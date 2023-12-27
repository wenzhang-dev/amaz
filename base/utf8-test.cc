#define CATCH_CONFIG_PREFIX_ALL
#include "utf8.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

namespace base {

CATCH_TEST_CASE("basic", "[utf8]") {
  char chs[] = "\xe7\xa0\x81\xe5\xb0\x98";

  std::uint32_t codepoint;
  auto len = Utf8DfaDecoder::Decode(chs, sizeof(chs), &codepoint);
  CATCH_REQUIRE(len.has_value());
  CATCH_REQUIRE(*len == 3);
}

}  // namespace base

int main(int argc, char* argv[]) { return Catch::Session().run(argc, argv); }
