#include "basic.h"

namespace base {
namespace json {

const Error::Category* Cat() {
  static JsonCategory kC;
  return &kC;
}

}  // namespace json
}  // namespace base
