#pragma once

#include <base/check.h>
#include <base/type-trait.h>

#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace base {
namespace json {

class Value;
class Directory;

namespace _ {

template <typename T>
inline constexpr bool IsJsonValue = std::is_same_v<RemoveCVRef<T>, Value>;

}

class List {
 public:
  List() = default;

  List(List&&) = default;
  List(const List&) = default;
  List& operator=(List&&) = default;
  List& operator=(const List&) = default;

 public:
  using iterator = std::vector<Value>::iterator;
  using const_iterator = std::vector<Value>::const_iterator;

  iterator begin() { return list_.begin(); }
  const_iterator begin() const { return list_.begin(); }

  iterator end() { return list_.end(); }
  const_iterator end() const { return list_.end(); }

  const_iterator cbegin() const { return list_.cbegin(); }
  const_iterator cend() const { return list_.cend(); }

  inline void Iteate(std::function<bool(Value&)> handler);
  inline void Iteate(std::function<bool(const Value&)> handler) const;

 public:
  inline void Append(bool);
  inline void Append(int);
  inline void Append(double);

  inline void Append(const char*);
  inline void Append(std::string_view);
  inline void Append(std::string&&);

  inline void Append(List&&);
  inline void Append(const List&);

  inline void Append(Directory&&);
  inline void Append(const Directory&);

  inline void Append(Value&&);
  inline void Append(const Value&);

  iterator Remove(iterator itr) { return list_.erase(itr); }
  iterator Remove(const_iterator itr) { return list_.erase(itr); }

  void Clear() { list_.clear(); }
  std::size_t Size() const { return list_.size(); }

 public:
  Value& operator[](std::size_t idx) {
    DCHECK(idx < Size());
    return list_[idx];
  }
  const Value& operator[](std::size_t idx) const {
    DCHECK(idx < Size());
    return list_[idx];
  }

  operator bool() const { return !list_.empty(); }

 private:
  std::vector<Value> list_;
};

class Directory {
 public:
  Directory() = default;

  Directory(Directory&&) = default;
  Directory(const Directory&) = default;
  Directory& operator=(Directory&&) = default;
  Directory& operator=(const Directory&) = default;

 public:
  using iterator = std::map<std::string, Value>::iterator;
  using const_iterator = std::map<std::string, Value>::const_iterator;

  iterator begin() { return map_.begin(); }
  const_iterator begin() const { return map_.begin(); }

  iterator end() { return map_.end(); }
  const_iterator end() const { return map_.end(); }

  const_iterator cbegin() const { return map_.cbegin(); }
  const_iterator cend() const { return map_.cend(); }

  inline void Iterate(std::function<bool(std::string_view, Value&)>);

  inline void Iterate(
      std::function<bool(std::string_view, const Value&)>) const;

 public:
  inline void Insert(std::string_view key, bool);
  inline void Insert(std::string_view key, int);
  inline void Insert(std::string_view key, double);

  inline void Insert(std::string_view key, const char* str);
  inline void Insert(std::string_view key, std::string_view str);
  inline void Insert(std::string_view key, std::string&& str);

  inline void Insert(std::string_view key, List&&);
  inline void Insert(std::string_view key, const List&);

  inline void Insert(std::string_view key, Directory&&);
  inline void Insert(std::string_view key, const Directory&);

  inline void Insert(std::string_view key, Value&&);
  inline void Insert(std::string_view key, const Value&);

  iterator Remove(iterator itr) { return map_.erase(itr); }
  iterator Remove(const_iterator itr) { return map_.erase(itr); }

  iterator Find(std::string_view key) { return map_.find(key); }

  void Clear() { map_.clear(); }
  std::size_t Size() const { return map_.size(); }

 public:
  inline bool InsertIfNotFound(std::string_view, bool);
  inline bool InsertIfNotFound(std::string_view, int);
  inline bool InsertIfNotFound(std::string_view, double);

  inline bool InsertIfNotFound(std::string_view, const char*);
  inline bool InsertIfNotFound(std::string_view, std::string_view);
  inline bool InsertIfNotFound(std::string_view, std::string&&);

  inline bool InsertIfNotFound(std::string_view, List&&);
  inline bool InsertIfNotFound(std::string_view, const List&);

  inline bool InsertIfNotFound(std::string_view, Directory&&);
  inline bool InsertIfNotFound(std::string_view, const Directory&);

  inline bool InsertIfNotFound(std::string_view, Value&&);
  inline bool InsertIfNotFound(std::string_view, const Value&);

 public:
  inline Value& operator[](std::string_view key);
  inline Value& operator[](const std::string& key) {
    return operator[](std::string_view(key));
  }
  inline Value& operator[](const char* key) {
    return operator[](std::string_view(key));
  }

  operator bool() const { return !map_.empty(); }

 private:
  // std::less<> mainly is used to compare between string_view and string
  // as a result, we can use string_view as key to insert and find elements
  std::map<std::string, Value, std::less<>> map_;
};

class Value {
 public:
  struct Null {};
  using Boolean = bool;
  using String = std::string;
  using Int = int;
  using Float = double;
  using Array = ::base::json::List;
  using Object = ::base::json::Directory;

  template <typename T>
  static inline constexpr bool IsValueType =
      std::is_same_v<T, Null> || std::is_same_v<T, Boolean> ||
      std::is_same_v<T, Int> || std::is_same_v<T, Float> ||
      std::is_same_v<T, String> || std::is_same_v<T, List> ||
      std::is_same_v<T, Directory>;

  enum Type : std::uint8_t {
    kNull = 0,
    kBoolean,
    kInt,
    kFloat,
    kString,
    kList,
    kDirectory,
  };

  // order matter
  using Storage =
      std::variant<Null, bool, int, double, std::string, List, Directory>;

 public:
  Value() = default;

  Value(Value&&) = default;
  Value(const Value&) = default;
  Value& operator=(Value&&) = default;
  Value& operator=(const Value&) = default;

  Value(bool b) : storage_(b) {}
  Value(int i) : storage_(i) {}
  Value(double d) : storage_(d) {}

  Value(const char* str) : storage_(std::string(str)) {}
  Value(std::string_view str) : storage_(std::string(str.begin(), str.end())) {}
  Value(std::string&& str) : storage_(std::move(str)) {}

  Value(List&& lst) : storage_(std::move(lst)) {}
  Value(const List& lst) : storage_(lst) {}

  Value(Directory&& dict) : storage_(std::move(dict)) {}
  Value(const Directory& dict) : storage_(dict) {}

  Value(const Null& null) : storage_(null) {}

 public:
  bool IsNull() const { return storage_.index() == kNull; }
  bool IsBoolean() const { return storage_.index() == kBoolean; }
  bool IsInt() const { return storage_.index() == kInt; }
  bool IsFloat() const { return storage_.index() == kFloat; }
  bool IsNumber() const { return IsInt() || IsFloat(); }
  bool IsString() const { return storage_.index() == kString; }
  bool IsList() const { return storage_.index() == kList; }
  bool IsDirectory() const { return storage_.index() == kDirectory; }

  operator bool() const { return !IsNull(); }

 public:
  bool* ToBool() {
    return IsBoolean() ? std::get_if<kBoolean>(&storage_) : nullptr;
  }
  const bool* ToBool() const {
    return IsBoolean() ? std::get_if<kBoolean>(&storage_) : nullptr;
  }

  int* ToInt() { return IsInt() ? std::get_if<kInt>(&storage_) : nullptr; }
  const int* ToInt() const {
    return IsInt() ? std::get_if<kInt>(&storage_) : nullptr;
  }

  double* ToFloat() {
    return IsFloat() ? std::get_if<kFloat>(&storage_) : nullptr;
  }
  const double* ToFloat() const {
    return IsFloat() ? std::get_if<kFloat>(&storage_) : nullptr;
  }

  std::string* ToString() {
    return IsString() ? std::get_if<kString>(&storage_) : nullptr;
  }

  const std::string* ToString() const {
    return IsString() ? std::get_if<kString>(&storage_) : nullptr;
  }

  List* ToList() { return IsList() ? std::get_if<kList>(&storage_) : nullptr; }
  const List* ToList() const {
    return IsList() ? std::get_if<kList>(&storage_) : nullptr;
  }

  Directory* ToDirectory() {
    return IsDirectory() ? std::get_if<kDirectory>(&storage_) : nullptr;
  }
  const Directory* ToDirectory() const {
    return IsDirectory() ? std::get_if<kDirectory>(&storage_) : nullptr;
  }

  template <typename T>
  std::enable_if_t<IsValueType<T>, T&> As() {
    return std::get<T>(storage_);
  }

  template <typename T>
  std::enable_if_t<IsValueType<T>, const T&> As() const {
    return std::get<T>(storage_);
  }

 public:
  void Clear() { storage_ = Null{}; }

  List& AsList(List&& lst) {
    storage_ = std::move(lst);
    return As<Array>();
  }
  List& AsList(const List& lst = {}) {
    storage_ = lst;
    return As<Array>();
  }

  Directory& AsDirectory(Directory&& dict) {
    storage_ = std::move(dict);
    return As<Object>();
  }
  Directory& AsDirectory(const Directory& dict = {}) {
    storage_ = dict;
    return As<Object>();
  }

  Null& AsNull() {
    storage_ = Null{};
    return As<Null>();
  }

  bool& AsBool(bool b = false) {
    storage_ = b;
    return As<Boolean>();
  }

  int& AsInt(int num = 0) {
    storage_ = num;
    return As<Int>();
  }

  double& AsFloat(double num = 0) {
    storage_ = num;
    return As<Float>();
  }

  std::string& AsString(const char* s) { return AsString(std::string_view(s)); }

  std::string& AsString(std::string_view s) {
    storage_ = std::string(s.begin(), s.end());
    return As<String>();
  }

  std::string& AsString(std::string&& s = {}) {
    storage_ = std::move(s);
    return As<String>();
  }

  Type type() const { return static_cast<Type>(storage_.index()); }

 public:
  std::string ToJson(bool pretty_format = false) const;

  std::size_t Search(std::string_view pattern, std::function<bool(Value&)>) {
    return 0;
  }

 private:
  Storage storage_;
};

inline void List::Append(bool b) { list_.emplace_back(b); }
inline void List::Append(int i) { list_.emplace_back(i); }
inline void List::Append(double d) { list_.emplace_back(d); }

inline void List::Append(const char* str) { list_.emplace_back(str); }
inline void List::Append(std::string_view str) { list_.emplace_back(str); }
inline void List::Append(std::string&& str) {
  list_.emplace_back(std::move(str));
}

inline void List::Append(List&& lst) { list_.emplace_back(std::move(lst)); }
inline void List::Append(const List& lst) { list_.emplace_back(lst); }

inline void List::Append(Directory&& dict) {
  list_.emplace_back(std::move(dict));
}
inline void List::Append(const Directory& dict) { list_.emplace_back(dict); }

inline void List::Append(Value&& v) { list_.push_back(std::move(v)); }
inline void List::Append(const Value& v) { list_.push_back(v); }

inline void Directory::Insert(std::string_view key, bool b) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = b;
  } else {
    map_.emplace(std::make_pair(key, b));
  }
}
inline void Directory::Insert(std::string_view key, int i) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = i;
  } else {
    map_.emplace(std::make_pair(key, i));
  }
}
inline void Directory::Insert(std::string_view key, double d) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = d;
  } else {
    map_.emplace(std::make_pair(key, d));
  }
}

inline void Directory::Insert(std::string_view key, const char* str) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = std::string(str);
  } else {
    map_.emplace(std::make_pair(key, str));
  }
}
inline void Directory::Insert(std::string_view key, std::string_view str) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = std::string(str.begin(), str.end());
  } else {
    map_.emplace(std::make_pair(key, str));
  }
}
inline void Directory::Insert(std::string_view key, std::string&& str) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = std::move(str);
  } else {
    map_.emplace(std::make_pair(key, std::move(str)));
  }
}

inline void Directory::Insert(std::string_view key, List&& lst) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = std::move(lst);
  } else {
    map_.emplace(std::make_pair(key, std::move(lst)));
  }
}
inline void Directory::Insert(std::string_view key, const List& lst) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = lst;
  } else {
    map_.emplace(std::make_pair(key, lst));
  }
}

inline void Directory::Insert(std::string_view key, Directory&& dict) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = std::move(dict);
  } else {
    map_.emplace(std::make_pair(key, std::move(dict)));
  }
}
inline void Directory::Insert(std::string_view key, const Directory& dict) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = dict;
  } else {
    map_.emplace(std::make_pair(key, dict));
  }
}

inline void Directory::Insert(std::string_view key, Value&& v) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = std::move(v);
  } else {
    map_.emplace(std::make_pair(key, std::move(v)));
  }
}
inline void Directory::Insert(std::string_view key, const Value& v) {
  if (auto itr = map_.find(key); itr != map_.end()) {
    itr->second = v;
  } else {
    map_.emplace(std::make_pair(key, v));
  }
}

inline bool Directory::InsertIfNotFound(std::string_view key, bool b) {
  auto [_, ok] = map_.emplace(std::make_pair(key, b));
  return ok;
}
inline bool Directory::InsertIfNotFound(std::string_view key, int i) {
  auto [_, ok] = map_.emplace(std::make_pair(key, i));
  return ok;
}
inline bool Directory::InsertIfNotFound(std::string_view key, double d) {
  auto [_, ok] = map_.emplace(std::make_pair(key, d));
  return ok;
}

inline bool Directory::InsertIfNotFound(std::string_view key, const char* str) {
  auto [_, ok] = map_.emplace(std::make_pair(key, str));
  return ok;
}
inline bool Directory::InsertIfNotFound(std::string_view key,
                                        std::string_view str) {
  auto [_, ok] = map_.emplace(std::make_pair(key, str));
  return ok;
}
inline bool Directory::InsertIfNotFound(std::string_view key,
                                        std::string&& str) {
  auto [_, ok] = map_.emplace(std::make_pair(key, std::move(str)));
  return ok;
}

inline bool Directory::InsertIfNotFound(std::string_view key, List&& lst) {
  auto [_, ok] = map_.emplace(std::make_pair(key, std::move(lst)));
  return ok;
}
inline bool Directory::InsertIfNotFound(std::string_view key, const List& lst) {
  auto [_, ok] = map_.emplace(std::make_pair(key, lst));
  return ok;
}

inline bool Directory::InsertIfNotFound(std::string_view key,
                                        Directory&& dict) {
  auto [_, ok] = map_.emplace(std::make_pair(key, std::move(dict)));
  return ok;
}
inline bool Directory::InsertIfNotFound(std::string_view key,
                                        const Directory& dict) {
  auto [_, ok] = map_.emplace(std::make_pair(key, std::move(dict)));
  return ok;
}

inline bool Directory::InsertIfNotFound(std::string_view key, Value&& v) {
  auto [_, ok] = map_.insert(std::make_pair(key, std::move(v)));
  return ok;
}

inline bool Directory::InsertIfNotFound(std::string_view key, const Value& v) {
  auto [_, ok] = map_.insert(std::make_pair(key, v));
  return ok;
}

inline Value& Directory::operator[](std::string_view key) {
  // operator[] may insert a new element if key not found
  // however, the map::emplace method will insert a new element if key not
  // found, otherwise, do nothing.
  // for both of them, this method always returns a iterator pointing to value
  // corresponding to the specific key
  auto [itr, _] = map_.emplace(std::make_pair(key, Value{}));
  return itr->second;
}

inline void Directory::Iterate(
    std::function<bool(std::string_view, Value&)> handler) {
  for (auto& [key, v] : map_) {
    if (!handler(key, v)) {
      return;
    }
  }
}

inline void Directory::Iterate(
    std::function<bool(std::string_view, const Value&)> handler) const {
  for (const auto& [key, v] : map_) {
    if (!handler(key, v)) {
      return;
    }
  }
}

inline void List::Iteate(std::function<bool(Value&)> handler) {
  for (auto& elem : list_) {
    if (!handler(elem)) {
      return;
    }
  }
}

inline void List::Iteate(std::function<bool(const Value&)> handler) const {
  for (const auto& elem : list_) {
    if (!handler(elem)) {
      return;
    }
  }
}

}  // namespace json
}  // namespace base
