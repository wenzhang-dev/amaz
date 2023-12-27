#pragma once

#include <base/type-trait.h>

#include <array>
#include <string_view>
#include <tuple>

namespace base {
namespace json {

namespace _ {

template <typename F>
constexpr inline std::size_t IterateArg(std::string_view s, F&& f,
                                        char delim = ',') {
  auto find_first_not = [s, delim](std::size_t pos = 0) -> std::size_t {
    for (std::size_t i = pos; i < s.size(); i++) {
      if (s[i] != delim) return i;
    }
    return std::string_view::npos;
  };

  std::size_t n = 0;
  for (std::size_t last_pos = find_first_not(0), pos = s.find(delim, last_pos);
       last_pos != std::string_view::npos;
       ++n, last_pos = find_first_not(pos), pos = s.find(delim, last_pos)) {
    if (!f(s.substr(last_pos, pos - last_pos))) break;
  }
  return n;
}

constexpr inline std::size_t CountArg(std::string_view s, char delim = ',') {
  auto f = [](const auto& arg) -> bool { return true; };
  return IterateArg(s, f, delim);
}

template <size_t N>
constexpr inline std::optional<std::array<std::string_view, N>> ParseArg(
    std::string_view name, std::string_view s, char delim = ',') {
  std::size_t i = 0;
  std::array<std::string_view, N> res;
  auto trim_and_validate = [&](const std::string_view& arg) -> bool {
    auto trim = [&]() -> std::string_view {
      auto begin = arg.find_first_not_of(" ");
      auto end = arg.find_last_not_of(" ");

      if (begin == std::string_view::npos) return {};
      return std::string_view(&arg[begin], end - begin + 1);
    };

    auto validate = [&](std::string_view s) -> bool {
      if (s.size() <= name.size() + 3 || s[0] != '&') return false;
      if (std::string_view(&s[1], name.size()) != name) return false;
      return std::string_view(&s[name.size() + 1], 2) == "::";
    };

    auto trim_str = trim();
    if (!validate(trim_str)) {
      return false;
    }

    auto actual_size = trim_str.size() - (name.size() + 3);
    res[i++] = std::string_view(&trim_str[name.size() + 3], actual_size);
    return true;
  };

  // Notes, if the size of parsed result don't equal to N, the result will fail
  if (N == IterateArg(s, trim_and_validate, delim)) {
    return res;
  }
  return {};
}

}  // namespace _

// clang-format off
#define MK_NAME(...) #__VA_ARGS__,
#define MK_NAME_VIEW(...) std::string_view{MK_NAME(__VA_ARGS__)}

#define ARG_COUNT(...) (::base::json::_::CountArg(MK_NAME_VIEW(__VA_ARGS__)))
// clang-format on

#define MK_META_DATA_IMPL(name, N, ...)                                      \
  [[maybe_unused]] inline static auto reflect_members(const name&) {         \
    struct reflect_members {                                                 \
      static_assert(field_arr_##name.has_value(), #name " ill-format");      \
      using size_type = std::integral_constant<std::size_t, N>;              \
      constexpr static decltype(auto) field_ptr() {                          \
        return std::make_tuple(__VA_ARGS__);                                 \
      }                                                                      \
      constexpr static std::size_t field_size() { return size_type::value; } \
      constexpr static std::array<std::string_view, N> field_name() {        \
        return *field_arr_##name;                                            \
      }                                                                      \
      constexpr static std::string_view field_name(std::size_t i) {          \
        return (*field_arr_##name)[i];                                       \
      }                                                                      \
    };                                                                       \
    return reflect_members{};                                                \
  }

#define MK_META_DATA(name, N, ...)                                        \
  static constexpr inline std::optional<std::array<std::string_view, N>>  \
      field_arr_##name =                                                  \
          ::base::json::_::ParseArg<N>(#name, MK_NAME_VIEW(__VA_ARGS__)); \
  MK_META_DATA_IMPL(name, N, __VA_ARGS__)

#define REFLECTION(name, ...) \
  MK_META_DATA(name, ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

namespace _ {

template <typename T, typename = void>
struct IsReflectionImpl : std::false_type {};

template <typename T>
struct IsReflectionImpl<
    T,
    std::void_t<decltype(std::declval<T>().reflect_members(std::declval<T>()))>>
    : std::true_type {};

}  // namespace _

template <typename T>
inline constexpr bool IsReflectable = _::IsReflectionImpl<T>::value;

template <typename T, typename = std::enable_if_t<IsReflectable<T>, int>>
inline auto ReflectType(const T& t) {
  return t.reflect_members(t);
}

template <typename... Args>
inline auto ReflectFieldType(std::tuple<Args...>) {
  return std::variant<Args...>{};
}

template <typename... Args, typename F, std::size_t... I>
constexpr void ForEach(std::tuple<Args...>& t, F&& f,
                       std::index_sequence<I...>) {
  (std::forward<F>(f)(std::get<I>(t), std::integral_constant<size_t, I>{}),
   ...);
}

template <typename... Args, typename F, std::size_t... I>
constexpr void ForEach(const std::tuple<Args...>& t, F&& f,
                       std::index_sequence<I...>) {
  (std::forward<F>(f)(std::get<I>(t), std::integral_constant<size_t, I>{}),
   ...);
}

template <typename T, typename F>
constexpr std::enable_if_t<IsReflectable<T>> ForEach(T&& t, F&& f) {
  using R = decltype(ReflectType(std::forward<T>(t)));
  ForEach(R::field_ptr(), std::forward<F>(f),
          std::make_index_sequence<R::field_size()>{});
}

template <typename T, typename F>
constexpr std::enable_if_t<IsTuple<std::decay_t<T>>> ForEach(T&& t, F&& f) {
  constexpr const std::size_t size = std::tuple_size_v<std::decay_t<T>>;
  ForEach(std::forward<T>(t), std::forward<F>(f),
          std::make_index_sequence<size>{});
}

}  // namespace json
}  // namespace base
