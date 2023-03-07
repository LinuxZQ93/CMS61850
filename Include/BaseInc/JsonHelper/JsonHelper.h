//
// Created by yichao on 2020/12/12.
//

#ifndef RK3399_AI_UTILS_INCLUDE_JSONHELPER_H_
#define RK3399_AI_UTILS_INCLUDE_JSONHELPER_H_

#include "json/json.h"
#include <stdexcept>
#include <utility>
#include <stdio.h>
#include <stdarg.h>
#include <functional>
#include <memory>

namespace ns_utils {

namespace __type_helper {

struct unknown_type {
  static_assert("not support this type");
};
struct int_type {};
struct unsigned_int_type {};
struct int64_type {};
struct uint64_type {};
struct string_type {};
struct float_type {};
struct double_type {};
struct bool_type {};


template<typename T, typename U>
struct is_same_type : unknown_type {
  bool value;
};

template<typename T>
struct is_same_type<T, int> : int_type { bool value; };
template<typename T>
struct is_same_type<T, unsigned int> : unsigned_int_type { bool value; };
template<typename T>
struct is_same_type<T, int64_t> : int64_type { bool value; };
template<typename T>
struct is_same_type<T, uint64_t> : uint64_type { bool value; };
template<typename T>
struct is_same_type<T, std::string> : string_type { bool value; };
template<typename T>
struct is_same_type<T, float> : float_type { bool value; };
template<typename T>
struct is_same_type<T, double> : double_type { bool value; };
template<typename T>
struct is_same_type<T, bool> : bool_type { bool value; };

}


class JsonException : public std::exception {
 public:
  explicit JsonException(const std::string& str) : M(str) {}

  ~JsonException() override = default;

  const char* what() const noexcept override {
    return M.what();
  }
 private:
  std::runtime_error M;
};

class JsonHelper {
 public:
  static std::string format(const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    return buf;
  }

  template<typename T>
  static bool fromJson(const Json::Value &value, T &type, std::string& err) {
    try {
      type.fromJson(value);
      return true;
    } catch(JsonException& e) {
      err = e.what();
      return false;
    }
  }

  template<typename T>
  static bool fromJson(const Json::Value &value, T &type) {
    try {
      type.fromJson(value);
      return true;
    } catch(JsonException& e) {
      return false;
    }
  }

  template<typename T>
  static void toJson(const T& type, Json::Value& value) {
    type.toJson(value);
  }

  // get value from a json object
  // the value type should be match the 3rd parameter, otherwise an exception will be thrown
  // you can also specify an validator routine to validate the read value, if validation is failed, an exception is thrown
  template <typename T>
  static void getValue(const Json::Value& json, const std::string& name, T& value,
                       const std::function<bool(const T&)> validator = std::function<bool(const T&)>(),
                       const std::string& err_string = "") {
    if (!json.isObject())
      throw ns_utils::JsonException("it's not an json object");
    if (!json.isMember(name))
      throw ns_utils::JsonException(format("'%s' not found", name.c_str()));
    innerGetValue(json[name], name, value, __type_helper::is_same_type<T, T>());
    if (validator && !validator(value)) {
      if (err_string.empty())
        throw ns_utils::JsonException(format("'%s' : value validation failed", name.c_str()));
      else
        throw ns_utils::JsonException(format("'%s' : value validation failed : %s", name.c_str(), err_string.c_str()));
    }
  }

  static void getStringNotEmpty(const Json::Value& json, const std::string& name, std::string& value) {
    if (!json.isMember(name))
      throw ns_utils::JsonException(format("'%s' not found", name.c_str()));
    innerGetValue(json[name], name, value, __type_helper::is_same_type<std::string, std::string>());
    if (value.empty())
      throw ns_utils::JsonException(format("'%s' : empty string is not valid", name.c_str()));
  }

//  template <typename T>
//  static void getValue(const Json::Value& json, const std::string& name, std::string& value, bool allow_empty) {
//    if (!json.isMember(name))
//      throw ns_utils::JsonException(format("'%s' not found", name.c_str()));
//    innerGetValue(json[name], name, value, __type_helper::is_same_type<std::string, std::string>());
//    if (!allow_empty && value.empty())
//      throw ns_utils::JsonException(format("'%s' : value validation failed", name.c_str()));
//  }

  // convert josn string to Json::Value object
  static bool stringToJson(const std::string& str, Json::Value& root) noexcept {
    if (str.empty()) return false;
    Json::CharReaderBuilder readerBuilder;
    JSONCPP_STRING errs;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    bool ret = reader->parse(str.c_str(), str.c_str() + str.length(), &root, &errs);
    return !(!ret || !errs.empty());
  }

 private:
  static void innerGetValue(const Json::Value& json, const std::string& name, int& value, const __type_helper::int_type&) {
    if (!json.isInt())
      throw ns_utils::JsonException(format("'%s' invalid type, expect int type", name.c_str()));
    value = json.asInt();
  }

  static void innerGetValue(const Json::Value& json, const std::string& name, unsigned int& value, const __type_helper::unsigned_int_type&) {
    if (!json.isUInt())
      throw ns_utils::JsonException(format("'%s' invalid type, expect unsigned int type", name.c_str()));
    value = json.asInt();
  }

  static void innerGetValue(const Json::Value& json, const std::string& name, int64_t& value, const __type_helper::int64_type&) {
    if (!json.isInt64())
      throw ns_utils::JsonException(format("'%s' invalid type, expect int64_t type", name.c_str()));
    value = json.asInt64();
  }

  static void innerGetValue(const Json::Value& json, const std::string& name, uint64_t& value, const __type_helper::uint64_type&) {
    if (!json.isUInt64())
      throw ns_utils::JsonException(format("'%s' invalid type, expect uint64_t type", name.c_str()));
    value = json.asUInt64();
  }

  static void innerGetValue(const Json::Value& json, const std::string& name, std::string& value, const __type_helper::string_type&) {
    if (!json.isString())
      throw ns_utils::JsonException(format("'%s' invalid type, expect string type", name.c_str()));
    value = json.asString();
  }

  static void innerGetValue(const Json::Value& json, const std::string& name, float& value, const __type_helper::float_type&) {
    if (!json.isDouble())
      throw ns_utils::JsonException(format("'%s' invalid type, expect float type", name.c_str()));
    value = json.asFloat();
  }

  static void innerGetValue(const Json::Value& json, const std::string& name, double& value, const __type_helper::double_type&) {
    if (!json.isDouble())
      throw ns_utils::JsonException(format("'%s' invalid type, expect double type", name.c_str()));
    value = json.asDouble();
  }

  static void innerGetValue(const Json::Value& json, const std::string& name, bool& value, const __type_helper::bool_type&) {
    if (!json.isBool())
      throw ns_utils::JsonException(format("'%s' invalid type, expect bool type", name.c_str()));
    value = json.asBool();
  }

};

}
#endif //RK3399_AI_UTILS_INCLUDE_JSONHELPER_H_
