#pragma once

#include <base/error.h>
#include <fmt/format.h>

#define JSON_ERROR_LIST(__)                  \
  __(kJsonErrorParseFalied, "json parser failed")

namespace base {
namespace json {

enum JsonError {
#define __(A, B) A,
  JSON_ERROR_LIST(__)
#undef __
};

struct JsonCategory : public Error::Category {
  const char* GetName() const override { return "json"; }
  std::string GetInformation(int c) const override {
    switch (c) {
#define __(A, B) \
  case A:        \
    return fmt::format("json[{}]", B);
      JSON_ERROR_LIST(__)
#undef __
      default:
        return "json[none]";
    }
  }
};

const Error::Category* Cat();

template <typename... Args>
inline Error Err(JsonError e, Args&&... args) {
  return Error{Cat(), e, std::forward<Args>(args)...};
}

}  // namespace json
}  // namespace base
