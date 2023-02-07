#pragma once

#include "nlohmann/json_fwd.hpp"
#include <string>
namespace krapi {

  template<typename Type>
  concept TaggableConcept = requires(Type t) {
    { t.tag() } -> std::same_as<std::string>;
  };
  template<typename Type>
  concept ConvertableToJson = requires(Type t, nlohmann::json j) {
    { Type::from_json(j) } -> std::same_as<Type>;
    { t.to_json() } -> std::same_as<nlohmann::json>;
  };

  template<typename T>
  concept DBComparable = requires(T t) {
    krapi::TaggableConcept<T>;
    { t.timestamp() } -> std::same_as<uint64_t>;
  };

  template<typename T>
  concept HasCreate = requires(T) { T::create(); };
}// namespace krapi