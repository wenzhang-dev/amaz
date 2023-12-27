#include "value.h"

#include "json-writer.h"
#include "pretty-formatter.h"

namespace base {
namespace json {
namespace {

template <bool Pretty>
inline std::string FormatJson(const Value& v) {
  Formatter<std::string, Pretty> formatter;
  ::base::json::ToJson(formatter, v);
  return std::move(formatter.stream());
}

}  // namespace

std::string Value::ToJson(bool pretty_format) const {
  return pretty_format ? FormatJson<true>(*this) : FormatJson<false>(*this);
}

}  // namespace json
}  // namespace base
