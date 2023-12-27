#pragma once

#include <fmt/format.h>
#include <fmt/printf.h>

#include <boost/intrusive/list.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <exception>
#include <optional>
#include <string>

#include "check.h"
#include "concept.h"
#include "macros.h"

#define NO_EXCEPT(...)                                                        \
  ::base::_::ExceptionRun<decltype(__VA_ARGS__)>(                             \
      [&]() -> decltype(__VA_ARGS__) { return (__VA_ARGS__); }, #__VA_ARGS__, \
      __FILE__, __LINE__)

#if defined(__COUNTER__)
#define __DEFERABLE_RUNNER_INST_NAME(arg) CONCATE(__defer, arg)
#define DEFERABLE_RUNNER_INST_NAME __DEFERABLE_RUNNER_INST_NAME(__COUNTER__)
#else
#define __DEFERABLE_RUNNER_INST_NAME(arg) CONCATE(__defer, arg)
#define DEFERABLE_RUNNER_INST_NAME __DEFERABLE_RUNNER_INST_NAME(__LINE__)
#endif

#define DEFER(...) \
  auto DEFERABLE_RUNNER_INST_NAME = ::base::_::MkDeferableRunner(__VA_ARGS__)

namespace base {

struct Dummy {};

// chrono
using NanoSeconds = std::chrono::nanoseconds;
using MicroSeconds = std::chrono::microseconds;
using MilliSeconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;
using Minutes = std::chrono::minutes;
using Hours = std::chrono::hours;

using SystemClock = std::chrono::system_clock;
using MonotonicClock = std::chrono::steady_clock;

template <class Clock, class Duration = typename Clock::duration>
using TimePoint = std::chrono::time_point<Clock, Duration>;

using Ts = TimePoint<SystemClock>;
using Tm = TimePoint<MonotonicClock>;

template <class Rep, class Period>
using Duration = std::chrono::duration<Rep, Period>;

template <class R, class Rep, class Period>
constexpr R DurationCast(const Duration<Rep, Period>& d) {
  return std::chrono::duration_cast<R>(d);
}

template <typename T>
std::optional<T> LexicalCast(std::string_view str) {
  T result;
  try {
    result = boost::lexical_cast<T>(str);
  } catch (boost::bad_lexical_cast&) {
    return std::nullopt;
  }
  return result;
}

// boost.intrusive_list
struct IntrusiveDefaultTag {};

template <typename Tag = IntrusiveDefaultTag,
          boost::intrusive::link_mode_type Link = boost::intrusive::auto_unlink>
using IntrusiveListBase =
    boost::intrusive::list_base_hook<boost::intrusive::tag<Tag>,
                                     boost::intrusive::link_mode<Link>>;

template <typename T, typename Tag = IntrusiveDefaultTag,
          boost::intrusive::link_mode_type Link = boost::intrusive::auto_unlink,
          bool ConstantTimeSize = false,
          typename BaseHook = IntrusiveListBase<Tag, Link>>
using IntrusiveList = boost::intrusive::list<
    T, boost::intrusive::base_hook<BaseHook>,
    boost::intrusive::constant_time_size<ConstantTimeSize>>;

template <typename F, typename... Args>
inline int SyscallWithoutINTR(F fn, Args&&... args) {
retry:
  int res = fn(std::forward<Args>(args)...);
  if (res == -1 && errno == EINTR) {
    goto retry;
  }
  return res;
}

[[noreturn]] inline void DieNow() noexcept { std::abort(); }
[[noreturn]] inline void Die() noexcept {
  // fmt::fprintf(stderr, "bt: \n{}\n", GetStackTrace());
  DieNow();
}

template <typename... ARGS>
[[noreturn]] inline void Die(const char* fmt, const ARGS&... args) {
  auto msg = fmt::format(fmt, args...);
  fmt::fprintf(stderr, "[die]: {}", msg);
  Die();
}

namespace _ {

template <typename R, typename F>
R ExceptionRun(F&& h, const char* expr, const char* file, int line) {
  try {
    return h();
  } catch (std::exception& e) {
    Die("%s.%d]: %s thrown exception %s", file, line, expr, e.what());
  } catch (...) {
    Die("%s.%d]: %s thrown unknown exception", file, line, expr);
  }
}

DEFINE_CONCEPT(HasStdSwap, void (_::*)(_&), &_::swap);
DEFINE_CONCEPT(HasUserSwap, void (_::*)(_*), &_::Swap);

template <typename T>
struct NoSwapConcept {
  static constexpr bool kValue =
      !HasUserSwap<T>::kValue && !HasStdSwap<T>::kValue;
};

template <typename T>
struct DeferableRunner {
  explicit DeferableRunner(T&& h) : runner(std::move(h)) {}
  ~DeferableRunner() { runner(); }

 private:
  T runner;
};

template <typename T>
struct TransactionRunner {
  explicit TransactionRunner(T&& h) : runner(std::move(h)), rollback(true) {}
  void Commit() { rollback = false; }
  ~TransactionRunner() {
    if (rollback) runner();
  }

 private:
  T runner;
  bool rollback;
};

template <typename T>
DeferableRunner<T> MkDeferableRunner(T&& h) {
  return DeferableRunner{std::move(h)};
}

}  // namespace _

template <typename T, std::enable_if_t<_::HasStdSwap<T>::kValue, int> _ = 0>
T Pass(T* t) noexcept {
  T tmp{};
  tmp.swap(*t);
  return tmp;
}

template <typename T, std::enable_if_t<_::HasUserSwap<T>::kValue, int> _ = 0>
T Pass(T* t) noexcept {
  T tmp{};
  tmp.Swap(t);
  return tmp;
}

template <typename T, std::enable_if_t<_::NoSwapConcept<T>::kValue, int> _ = 0>
T Pass(T* v) noexcept {
  T tmp{};
  std::swap(tmp, *v);
  return tmp;
}

template <typename T>
T Pass(std::optional<T>* v) {
  if (!v) Die("unref empty optional");
  DEFER([&]() { *v = std::nullopt; });
  return T{std::move(v->value())};
}

template <typename T>
T Pass(std::optional<T>& v) {
  if (!v) Die("undef empty optional");
  DEFER([&]() { v = std::nullopt; });
  return T{std::move(*v)};
}

template <typename T>
void DestoryInplace(T& ref) noexcept {
  ref.~T();
}

template <typename T, typename... ARGS>
void ConstructInplace(T& ref, ARGS&&... args) noexcept {
  ::new (std::addressof(ref)) T(std::forward(args)...);
}

}  // namespace base

