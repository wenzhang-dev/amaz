#pragma once

#include <fmt/format.h>
#include <base/common.h>
#include <base/type-trait.h>
#include <base/utf8.h>

#include <charconv>
#include <iostream>

#include "basic.h"
#include "reflection.h"
#include "value.h"

#define JSON_TOKEN_LIST(__) \
  __(kTColon, ":")          \
  __(kTComma, ",")          \
  __(kTLSqBracket, "[")     \
  __(kTRSqBracket, "]")     \
  __(kTLBrace, "{")         \
  __(kTRBrace, "}")         \
  __(kTStr, "str")          \
  __(kTNum, "num")          \
  __(kTBool, "bool")        \
  __(kTNull, "null")        \
  __(kTEof, "<eof>")        \
  __(kTError, "<error>")

namespace base {
namespace json {

namespace _ {

class JsonParser {
 public:
  enum Token : std::uint8_t {
#define __(A, B) A,
    JSON_TOKEN_LIST(__)
#undef __
  };

  static const char* TokenString(Token tk) {
    switch (tk) {
#define __(A, B) \
  case A:        \
    return B;
      JSON_TOKEN_LIST(__)
#undef __
    }
    return "unknown token";
  }

  struct Lexer {
    Lexer(std::string_view data) : data_(data) {}
    Lexer(Lexer&&) = default;
    Lexer& operator=(Lexer&&) = default;

    inline Token Next();
    inline bool Must(Token tk);
    bool MustNext(Token tk) { return Must(Next()); }
    Token token() { return token_; }
    std::string_view expr() { return expr_; }
    template <typename... Args>
    inline Token E(const char* fmt, Args&&... args);

    std::string PassExpr() { return std::move(expr_); }
    Token Ret(Token tk, const char* expr) {
      return Ret(tk, std::string_view(expr));
    }
    Token Ret(Token tk, std::string_view expr) {
      expr_ = expr;
      return (token_ = tk);
    }
    Token Ret(Token tk, std::string&& expr) {
      expr_ = std::move(expr);
      return (token_ = tk);
    }

    template <std::size_t N>
    bool Literal(const char (&arr)[N]) {
      // Notes, `N - 1` equals to string length
      std::string_view src(data_.begin() + cursor_, N - 1);
      std::string_view dst(arr, N - 1);
      if (src == dst) {
        cursor_ += (N - 1);
        return true;
      }
      return false;
    }

    inline Token LexStr();
    inline Token LexNum();
    inline Token LexBoolOrNull();

    bool IsEof() const { return token_ == kTEof; }
    bool IsError() const { return static_cast<bool>(err_); }
    bool IsControlToken() const { return IsEof() || IsError(); }
    Error error() { return err_; }

    inline bool NextChar();
    inline std::optional<std::uint32_t> PeekChar();

    Error err_;
    Token token_;
    std::uint32_t c0_{0};
    std::uint32_t c0_length_{0};
    std::size_t cursor_{0};
    std::string expr_;
    std::string_view data_;
  };

 public:
  JsonParser(std::string_view data) : lex_(data) {}

  JsonParser(JsonParser&&) = default;
  JsonParser& operator=(JsonParser&&) = default;

 public:
  template <typename T, std::enable_if_t<IsReflectable<T>, int> = 0>
  Error Parse(T& value) {
    using R = decltype(ReflectType(std::forward<T>(value)));
    using FieldType = decltype(ReflectFieldType(R::field_ptr()));

    std::map<std::string_view, FieldType, std::less<>> mapping;
    ForEach(value, [&mapping](const auto& v, auto i) {
      mapping.insert(std::make_pair(R::field_name(i), v));
    });

    lex_.MustNext(kTLBrace);
    lex_.Next();

    bool started = false;
    while (!lex_.IsControlToken()) {
      if (lex_.token() == kTRBrace) {
        lex_.MustNext(kTEof);
        break;
      }

      if (started) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      lex_.Must(kTStr);
      auto key = lex_.PassExpr();

      lex_.MustNext(kTColon);
      lex_.Next();

      // TODO: for compatibility, we may need ignore unknown fields
      if (auto itr = mapping.find(key); itr != mapping.end()) {
        std::visit(
            [&, this](auto&& member_ptr) { ParseItem(value.*member_ptr); },
            itr->second);
      } else {
        lex_.E("expect key `{}` but no found", key);
      }
      started = true;
    }
    return lex_.error();
  }

  template <typename T, std::enable_if_t<IsJsonValue<T>, int> = 0>
  Error Parse(T& value) {
    auto& dict = value.AsDirectory();

    lex_.MustNext(kTLBrace);
    lex_.Next();

    while (!lex_.IsControlToken()) {
      if (lex_.token() == kTRBrace) {
        lex_.MustNext(kTEof);
        break;
      }

      if (dict) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      lex_.Must(kTStr);
      auto key = lex_.PassExpr();

      lex_.MustNext(kTColon);
      lex_.Next();

      ParseJsonValue(dict[key]);
    }

    return lex_.error();
  }

 private:
  template <typename T, std::enable_if_t<IsJsonValue<T>, int> = 0>
  void ParseJsonValue(T& value) {
    auto is_floatpoint = [](std::string_view s) -> bool {
      for (const auto& ch : s) {
        if (ch == '.' || ch == 'e' || ch == 'E') {
          return true;
        }
      }
      return false;
    };

    switch (lex_.token()) {
      case kTBool:
        ParseItem(value.AsBool());
        break;
      case kTStr:
        ParseItem(value.AsString());
        break;
      case kTNum:
        if (is_floatpoint(lex_.expr()))
          ParseItem(value.AsFloat());
        else
          ParseItem(value.AsInt());
        break;
      case kTNull:
        value.AsNull();
        lex_.Next();
        break;
      case kTLBrace:
        ParseJsonObject(value.AsDirectory());
        break;
      case kTLSqBracket:
        ParseJsonArray(value.AsList());
        break;
      default:
        lex_.E("unexpected token `{}`", TokenString(lex_.token()));
        break;
    }
  }

  void ParseJsonArray(List& lst) {
    lex_.Must(kTLSqBracket);
    lex_.Next();

    Value value;
    lst.Clear();
    while (!lex_.IsControlToken()) {
      if (lex_.token() == kTRSqBracket) {
        lex_.Next();
        return;
      }

      if (lst) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      ParseJsonValue(value);
      lst.Append(std::move(value));
    }

    lex_.E("unexpected terminate");
  }

  void ParseJsonObject(Directory& dict) {
    lex_.Must(kTLBrace);
    lex_.Next();

    Value value;
    dict.Clear();
    while (!lex_.IsControlToken()) {
      if (lex_.token() == kTRBrace) {
        lex_.Next();
        return;
      }

      if (dict) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      lex_.Must(kTStr);
      auto key = lex_.PassExpr();

      lex_.MustNext(kTColon);
      lex_.Next();

      ParseJsonValue(value);
      dict.Insert(key, std::move(value));
    }

    lex_.E("unexpected terminate");
  }

 private:
  template <typename T, std::enable_if_t<IsBool<T>, int> = 0>
  void ParseItem(T& value) {
    if (lex_.Must(kTBool)) {
      value = lex_.expr() == "true";
      lex_.Next();
    }
  }

  template <typename T, std::enable_if_t<IsNumeric<T>, int> = 0>
  void ParseItem(T& value) {
    using U = std::decay_t<T>;
    if (lex_.Must(kTNum)) {
      if (auto opt = LexicalCast<U>(lex_.expr()); opt) {
        value = opt.value();
        lex_.Next();
      } else {
        lex_.E("expect `num` but got `{}`", lex_.expr());
      }
    }
  }

  template <typename T, std::enable_if_t<IsChar<T>, int> = 0>
  void ParseItem(T& value) {
    if (lex_.Must(kTStr)) {
      auto expr = lex_.expr();
      std::uint32_t codepoint = 0;
      if (auto opt =
              Utf8DfaDecoder::Decode(expr.data(), expr.size(), &codepoint);
          opt) {
        if (opt.value() == expr.size()) {
          value = codepoint;
          lex_.Next();
          return;
        }
      }
      lex_.E("unexpected char");
    }
  }

  template <typename T, std::enable_if_t<IsString<T>, int> = 0>
  void ParseItem(T& value) {
    if (lex_.Must(kTStr)) {
      using U = std::decay_t<T>;
      if constexpr (std::is_same_v<std::string, U>) {
        value = std::move(lex_.PassExpr());
      } else {
        value.clear();
        auto expr = lex_.expr();
        value.reserve(expr.size());
        for (const auto& ch : expr) {
          value.push_back(ch);
        }
      }
      lex_.Next();
    }
  }

  template <typename T, std::enable_if_t<IsMapContainer<T>, int> = 0>
  void ParseItem(T& value) {
    using U = std::remove_reference_t<T>;
    using KeyType = typename U::key_type;

    value.clear();
    lex_.Must(kTLBrace);
    lex_.Next();
    while (!lex_.IsControlToken()) {
      if (lex_.token() == kTRBrace) {
        lex_.Next();
        return;
      }

      if (!value.empty()) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      lex_.Must(kTStr);
      auto key = lex_.PassExpr();

      lex_.MustNext(kTColon);
      lex_.Next();

      if constexpr (IsStringLike<KeyType>) {
        ParseItem(value[KeyType(key)]);
      } else if (IsNumeric<KeyType> || IsChar<KeyType>) {
        if (auto opt = LexicalCast<KeyType>(key); opt) {
          ParseItem(value[opt.value()]);
        } else {
          lex_.E("expect `num` but got `{}`", key);
        }
      } else {
        lex_.E("unsupport type");
      }
    }

    lex_.E("unexpected terminate");
  }

  template <typename T, std::enable_if_t<IsSequenceContainer<T>, int> = 0>
  void ParseItem(T& value) {
    lex_.Must(kTLSqBracket);
    lex_.Next();

    value.clear();
    while (!lex_.IsControlToken()) {
      if (lex_.token() == kTRSqBracket) {
        lex_.Next();
        return;
      }

      if (!value.empty()) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      ParseItem(value.emplace_back());
    }

    lex_.E("unexpected terminate");
  }

  template <typename T, std::enable_if_t<IsSetContainer<T>, int> = 0>
  void ParseItem(T& value) {
    using U = std::remove_reference_t<T>;
    using KeyType = typename U::key_type;

    lex_.Must(kTLSqBracket);
    lex_.Next();

    KeyType v{};
    value.clear();
    while (!lex_.IsControlToken()) {
      if (lex_.token() == kTRSqBracket) {
        lex_.Next();
        return;
      }

      if (!value.empty()) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      ParseItem(v);
      value.insert(std::move(v));
    }

    lex_.E("unexpected terminate");
  }

  template <typename T, std::enable_if_t<IsOptional<T>, int> = 0>
  void ParseItem(T& value) {
    using U = std::remove_reference_t<T>;
    using ValueType = typename U::value_type;

    if (lex_.token() == kTNull) {
      value = std::nullopt;
      lex_.Next();
    } else {
      ValueType v{};
      ParseItem(v);
      value = std::move(v);
    }
  }

  template <typename T, std::enable_if_t<IsSmartPtr<T>, int> = 0>
  void ParseItem(T& value) {
    using U = std::remove_reference_t<T>;
    using ValueType = typename U::element_type;

    if (lex_.token() == kTNull) {
      value = nullptr;
      lex_.Next();
    } else {
      if constexpr (IsUniquePtr<T>) {
        value = std::make_unique<ValueType>();
      } else {
        value = std::make_shared<ValueType>();
      }
      ParseItem(*value);
    }
  }

  template <typename T, std::enable_if_t<IsTuple<T>, int> = 0>
  void ParseItem(T& value) {
    lex_.Must(kTLSqBracket);
    lex_.Next();

    ForEach(value, [&, this](auto& v, auto i) {
      constexpr auto I = decltype(i)::value;
      if (lex_.token() == kTRSqBracket) {
        return;
      }

      if constexpr (I != 0) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      ParseItem(v);
    });

    if (lex_.token() == kTRSqBracket) {
      lex_.Next();
    } else {
      lex_.E("unexpected terminate");
    }
  }

  template <typename T, std::enable_if_t<IsCharArray<T>, int> = 0>
  void ParseItem(T& value) {
    using U = std::remove_reference_t<T>;
    constexpr std::size_t n =
        sizeof(U) / sizeof(decltype(std::declval<U>()[0]));

    if (lex_.token() == kTStr) {
      auto str = lex_.expr();
      if (str.size() > n) {
        lex_.E("array out of range");
      } else {
        std::size_t i = 0;
        for (const auto& ch : str) {
          value[i++] = ch;
        }
        lex_.Next();
      }
      return;
    }

    ParseArray(value);
  }

  template <typename T, std::enable_if_t<IsNonCharArray<T>, int> = 0>
  void ParseItem(T& value) {
    ParseArray(value);
  }

  template <typename T, std::enable_if_t<IsFixedArray<T>, int> = 0>
  void ParseArray(T& value) {
    using U = std::remove_reference_t<T>;
    constexpr std::size_t n =
        sizeof(U) / sizeof(decltype(std::declval<U>()[0]));

    lex_.Must(kTLSqBracket);
    lex_.Next();

    auto itr = std::begin(value);
    for (std::size_t i = 0; !lex_.IsControlToken(); ++i) {
      if (lex_.token() == kTRSqBracket) {
        lex_.Next();
        return;
      }

      if (i >= n) break;

      if (i > 0) {
        lex_.Must(kTComma);
        lex_.Next();
      }

      ParseItem(*itr++);
    }

    lex_.E("unexpected terminate");
  }

  Lexer lex_;

  DISALLOW_COPY_AND_ASSIGN(JsonParser);
};

template <typename... Args>
inline auto JsonParser::Lexer::E(const char* fmt, Args&&... args) -> Token {
  if (IsError()) return token();

  constexpr std::size_t kErrorContextSize = 32;
  std::size_t start_cursor = 0;
  if (cursor_ >= kErrorContextSize) start_cursor = cursor_ - kErrorContextSize;
  std::size_t end_cursor = cursor_ + kErrorContextSize;
  if (end_cursor > data_.size()) {
    end_cursor = data_.size();
  }
  std::string_view error_ctx(data_.begin() + start_cursor,
                             end_cursor - start_cursor);

  auto mesg = fmt::format(fmt, std::forward<Args>(args)...);
  err_ = Err(kJsonErrorParseFalied, fmt::format("* {} *: {}", error_ctx, mesg));
  return (token_ = kTError);
}

inline bool JsonParser::Lexer::Must(Token tk) {
  if (tk != token()) {
    E("expect `{}` but got `{}`", TokenString(tk), TokenString(token()));
  }
  return tk == token();
}

inline auto JsonParser::Lexer::Next() -> Token {
  if (IsError()) return token();

  while (cursor_ < data_.size()) {
    if (!NextChar()) {
      return E("utf8 decoder failed");
    }

    switch (c0_) {
      case '[':
        return Ret(kTLSqBracket, "[");
      case ']':
        return Ret(kTRSqBracket, "]");
      case '{':
        return Ret(kTLBrace, "{");
      case '}':
        return Ret(kTRBrace, "}");
      case ',':
        return Ret(kTComma, ",");
      case ':':
        return Ret(kTColon, ":");
      case '"':
        return LexStr();
      case ' ':
      case '\b':
      case '\v':
      case '\r':
      case '\t':
      case '\n':
        continue;
      case '.':
      case 'e':
      case 'E':
      case '0' ... '9':
        return LexNum();
      default:
        return LexBoolOrNull();
    }
  }

  return Ret(kTEof, "");
}

inline auto JsonParser::Lexer::LexStr() -> Token {
  std::string buf;
  auto parse_escape = [&](std::uint32_t codepoint) -> bool {
    switch (codepoint) {
      case 'v':
        buf.push_back('\v');
        break;
      case 't':
        buf.push_back('\t');
        break;
      case 'r':
        buf.push_back('\r');
        break;
      case 'n':
        buf.push_back('\n');
        break;
      case '\\':
        buf.push_back('\\');
        break;
      case '"':
        buf.push_back('"');
        break;
      case '\'':
        buf.push_back('\'');
        break;
      default:
        return false;
    }
    return true;
  };
  while (cursor_ < data_.size()) {
    if (!NextChar()) {
      return E("utf8 decoder failed");
    }

    if (c0_ == '"') {
      break;
    } else if (c0_ == '\\') {
      if (!NextChar()) {
        return E("utf8 decoder failed");
      }

      if (!parse_escape(c0_)) {
        return E("unknown escape `{}`", c0_);
      }
    } else {
      Utf8Encode(buf, c0_);
    }
  }

  if (c0_ != '"') {
    return E("early terminate in string literal");
  }

  return Ret(kTStr, std::move(buf));
}

inline auto JsonParser::Lexer::LexNum() -> Token {
  std::string buf;
  buf.push_back(c0_);

  auto is_numeric = [](std::uint32_t codepoint) -> bool {
    return (codepoint >= '0' && codepoint <= '9') || codepoint == 'E' ||
           codepoint == 'e' || codepoint == '.';
  };

  while (cursor_ < data_.size()) {
    if (auto opt = PeekChar(); !opt) {
      return E("utf8 decoder failed");
    } else if (is_numeric(opt.value())) {
      buf.push_back(opt.value());
      NextChar();
    } else {
      break;
    }
  }

  return Ret(kTNum, std::move(buf));
}

inline auto JsonParser::Lexer::LexBoolOrNull() -> Token {
  switch (c0_) {
    case 'n':
      if (Literal("ull")) return Ret(kTNull, "null");
      break;
    case 'f':
      if (Literal("alse")) return Ret(kTBool, "false");
      break;
    case 't':
      if (Literal("rue")) return Ret(kTBool, "true");
      break;
    default:
      break;
  }
  return E("unknown chars");
}

inline bool JsonParser::Lexer::NextChar() {
  std::size_t remain_size = data_.size() - cursor_;
  auto opt = Utf8DfaDecoder::Decode(&data_[cursor_], remain_size, &c0_);
  if (opt) {
    c0_length_ = opt.value();
    cursor_ += c0_length_;
  }
  return opt.has_value();
}

inline std::optional<std::uint32_t> JsonParser::Lexer::PeekChar() {
  std::uint32_t res;
  std::size_t remain_size = data_.size() - cursor_;
  auto opt = Utf8DfaDecoder::Decode(&data_[cursor_], remain_size, &res);
  if (opt) {
    return res;
  }
  return std::nullopt;
}

}  // namespace _

template <typename T, std::enable_if_t<IsReflectable<T>, int> = 0>
Error FromJson(std::string_view json_str, T& value) {
  _::JsonParser parser(json_str);
  return parser.Parse(value);
}

template <typename T, std::enable_if_t<_::IsJsonValue<T>, int> = 0>
Error FromJson(std::string_view json_str, T& value) {
  _::JsonParser parser(json_str);
  return parser.Parse(value);
}

}  // namespace json
}  // namespace base
