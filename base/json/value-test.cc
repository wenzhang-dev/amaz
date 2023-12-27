#define CATCH_CONFIG_PREFIX_ALL
#include "value.h"

#include <base/type-trait.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

#include <iostream>

namespace base {
namespace json {
namespace {

template <typename T>
inline constexpr bool IsJsonValue = std::is_same_v<RemoveCVRef<T>, Value>;

template <typename Stream, typename T,
          std::enable_if_t<IsJsonValue<T>, int> = 0>
inline void JsonType(Stream& s, T&& t) {
  switch (t.type()) {
    case Value::Type::kNull:
      s.append("null");
      break;
    case Value::Type::kBoolean:
      std::cout << std::is_same_v<bool&, decltype(t.template As<Value::Boolean>())> << std::endl;
      s.append("boolean");
      break;
    case Value::Type::kInt:
      s.append("int");
      break;
    case Value::Type::kFloat:
      s.append("float");
      break;
    case Value::Type::kString:
      s.append("string");
      break;
    case Value::Type::kList:
      s.append("array");
      break;
    case Value::Type::kDirectory:
      s.append("object");
      break;
  }
}

template <typename T>
std::string_view Signature() {
    return __PRETTY_FUNCTION__;
}

}  // namespace

CATCH_TEST_CASE("basic", "[value]") {
  {
    Value v;
    CATCH_REQUIRE(!v);
    CATCH_REQUIRE(v.IsNull());

    v = true;
    CATCH_REQUIRE(v);
    CATCH_REQUIRE(v.IsBoolean());
    CATCH_REQUIRE(v.As<Value::Boolean>());
    CATCH_REQUIRE(*v.ToBool());

    v = 123;
    CATCH_REQUIRE(v);
    CATCH_REQUIRE(v.IsInt());
    CATCH_REQUIRE(v.As<Value::Int>() == 123);
    CATCH_REQUIRE(*v.ToInt() == 123);

    v = 3.14f;
    CATCH_REQUIRE(v);
    CATCH_REQUIRE(v.IsFloat());
    CATCH_REQUIRE(v.As<Value::Float>() == 3.14f);
    CATCH_REQUIRE(*v.ToFloat() == 3.14f);

    v = "456";
    CATCH_REQUIRE(v);
    CATCH_REQUIRE(v.IsString());
    CATCH_REQUIRE(v.As<Value::String>() == "456");
    CATCH_REQUIRE(*v.ToString() == "456");

    {
      List lst;
      CATCH_REQUIRE(!lst);

      lst.Append(true);
      lst.Append(123);
      lst.Append(3.14f);
      lst.Append("456");

      CATCH_REQUIRE(lst.Size() == 4);
      CATCH_REQUIRE(lst[0].IsBoolean());
      CATCH_REQUIRE(lst[1].IsInt());
      CATCH_REQUIRE(lst[2].IsFloat());
      CATCH_REQUIRE(lst[3].IsString());

      for (auto& e : lst) {
        if (e.IsBoolean()) CATCH_REQUIRE(e.As<Value::Boolean>());
        if (e.IsInt()) CATCH_REQUIRE(e.As<Value::Int>() == 123);
        if (e.IsFloat()) CATCH_REQUIRE(e.As<Value::Float>() == 3.14f);
        if (e.IsString()) CATCH_REQUIRE(e.As<Value::String>() == "456");
      }

      Value v;
      CATCH_REQUIRE(!v);

      auto& new_lst = v.AsList(lst);
      CATCH_REQUIRE(v);
      CATCH_REQUIRE(new_lst.Size() == 4);
      CATCH_REQUIRE(v.IsList());

      v.Clear();
      CATCH_REQUIRE(!v);
    }

    {
      Directory dict;
      CATCH_REQUIRE(!dict);

      dict.Insert("001", 123);
      CATCH_REQUIRE(dict);
      CATCH_REQUIRE(dict.Size() == 1);
      CATCH_REQUIRE(dict["001"].As<Value::Int>() == 123);

      dict.Insert("001", 456);
      CATCH_REQUIRE(dict["001"].As<Value::Int>() == 456);

      CATCH_REQUIRE(!dict.InsertIfNotFound("001", 789));
      CATCH_REQUIRE(dict["001"].As<Value::Int>() == 456);

      dict.Insert("002", true);
      CATCH_REQUIRE(dict.Size() == 2);
      CATCH_REQUIRE(dict["002"].As<Value::Boolean>());

      dict.Insert("003", 456);
      CATCH_REQUIRE(dict.Size() == 3);
      CATCH_REQUIRE(dict["003"].As<Value::Int>() == 456);

      dict.Insert("004", 3.14f);
      dict.Insert("005", "hello");
      CATCH_REQUIRE(dict.Size() == 5);

      {
        List lst;
        lst.Append("123");
        lst.Append(123);

        dict.Insert("006", std::move(lst));
        CATCH_REQUIRE(dict.Size() == 6);
      }

      {
        Directory dict1;
        dict1["123"] = 123;

        dict.Insert("007", std::move(dict1));
        CATCH_REQUIRE(dict.Size() == 7);
      }

      for (auto& [k, v] : dict) {
        if (k == "001") CATCH_REQUIRE(v.As<Value::Int>() == 456);
        if (k == "002") CATCH_REQUIRE(v.As<Value::Boolean>());
        if (k == "003") CATCH_REQUIRE(v.As<Value::Int>() == 456);
        if (k == "004") CATCH_REQUIRE(v.As<Value::Float>() == 3.14f);
        if (k == "005") CATCH_REQUIRE(v.As<Value::String>() == "hello");
      }

      dict.Iterate([](std::string_view k, Value& v) -> bool {
        if (k == "001") CATCH_REQUIRE(v.As<Value::Int>() == 456);
        if (k == "002") CATCH_REQUIRE(v.As<Value::Boolean>());
        if (k == "003") CATCH_REQUIRE(v.As<Value::Int>() == 456);
        if (k == "004") CATCH_REQUIRE(v.As<Value::Float>() == 3.14f);
        if (k == "005") CATCH_REQUIRE(v.As<Value::String>() == "hello");

        return true;
      });

      dict.Clear();
      CATCH_REQUIRE(!dict);
      CATCH_REQUIRE(dict.Size() == 0);
    }
  }

  {
    Value v(true);
    std::string type;
    JsonType(type, v);
    CATCH_REQUIRE(type == "boolean");

    v = 123;
    int& ref = v.As<Value::Int>();
    ref = 789;


    std::cout << Signature<decltype(v.As<Value::Int>())>() << std::endl;

    
    std::cout << v.As<Value::Int>() << std::endl;

    const Value v1(111);
    std::cout << IsIntegral<decltype(v1.As<Value::Int>())> << std::endl;
    std::cout << Signature<decltype(v1.As<Value::Int>())>() << std::endl;
  }

  {
    Value v;
    auto& dict = v.AsDirectory();

    dict["001"] = 123;
    dict["002"] = "123";
    dict["003"] = false;
    dict["004"] = 3.14f;
    dict["005"] = Value::Null{};

    dict["006"] = dict;

    {
        List lst;
        lst.Append(123);
        lst.Append(true);
        lst.Append(3.14f);

        dict["007"] = lst;
    }

    std::cout << v.ToJson() << std::endl;

    std::cout << v.ToJson(true) << std::endl;
  }
}

}  // namespace json
}  // namespace base

int main(int argc, char* argv[]) { return Catch::Session().run(argc, argv); }
