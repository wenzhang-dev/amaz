#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>

namespace base {

inline std::string ToLower(std::string_view s) {
  std::string result;
  result.resize(s.size());
  std::transform(s.begin(), s.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

inline std::string& ToLowerInplace(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

inline std::string ToUpper(std::string_view s) {
  std::string result;
  result.resize(s.size());
  std::transform(s.begin(), s.end(), result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

inline std::string& ToUpper(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return s;
}

inline bool StartsWith(std::string_view s, std::string_view prefix) {
  return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

inline bool EndsWith(std::string_view s, std::string_view suffix) {
  return s.size() >= suffix.size() &&
         s.substr(s.size() - suffix.size()) == suffix;
}

inline std::string_view LTrim(std::string_view s) {
  int idx = 0, size = static_cast<int>(s.size());
  for (; idx < size && s[idx] == ' '; ++idx)
    ;
  return idx >= size ? std::string_view{}
                     : std::string_view(&s[idx], size - idx);
}

inline std::string_view RTrim(std::string_view s) {
  int size = static_cast<int>(s.size()), idx = size - 1;
  for (; idx >= 0 && s[idx] == ' '; --idx)
    ;
  return idx < 0 ? std::string_view{}
                 : std::string_view{&s[0], static_cast<std::size_t>(idx + 1)};
}

inline std::string_view Trim(std::string_view s) {
  int l_idx = 0, size = static_cast<int>(s.size());
  for (; l_idx < size && s[l_idx] == ' '; ++l_idx)
    ;

  int r_idx = size - 1;
  for (; r_idx > l_idx && s[r_idx] == ' '; --r_idx)
    ;

  return l_idx > r_idx ? std::string_view{}
                       : std::string_view(&s[l_idx], r_idx - l_idx + 1);
}

// if `skip_null` is true, all empty string results will be filtered
//   eg. `a,b,c,`, the result is [`a`, `b`, `c`] when the flag is set.
//   otherwise, the result is [`a`, `b`, `c`, ``]
inline std::vector<std::string_view> Tokenize(std::string_view str,
                                              std::string_view delimiter = ",",
                                              bool skip_null = true) {
  std::vector<std::string_view> result;
  auto inserter = [skip_null](std::vector<std::string_view>& result,
                              std::string_view elem) {
    if (skip_null && elem.empty()) {
      return;
    }
    result.push_back(elem);
  };

  if (delimiter == str) {
    return result;
  }

  if (delimiter.empty() || str.size() <= delimiter.size()) {
    inserter(result, str);
    return result;
  }

  std::size_t start = 0;
  std::size_t loop_size = str.size() - delimiter.size();
  for (std::size_t i = 0; i <= loop_size; ++i) {
    std::size_t j = 0;
    for (; j < delimiter.size(); ++j) {
      if (str[i + j] != delimiter[j]) {
        break;
      }
    }

    if (j == delimiter.size()) {
      inserter(result, std::string_view(&str[start], i - start));
      start = i + j;
    }
  }

  // handle the remaining data
  if (start != loop_size + 1) {
    inserter(result, std::string_view(&str[start], str.size() - start));
  }

  // the suffix equals to the delimiter string
  // when tokenize `12,` with `,` delimiter, we expect the result is ["12", ""]
  std::string_view suffix(&str[loop_size], delimiter.size());
  if (start == loop_size + 1 && delimiter == suffix) {
    inserter(result, std::string_view{});
  }

  return result;
}

}  // namespace base
