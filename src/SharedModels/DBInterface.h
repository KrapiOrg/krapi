#pragma once
#include "fmt/format.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/iterator.h"
#include "nlohmann/json.hpp"
#include <concurrencpp/errors.h>
#include <concurrencpp/results/generator.h>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

template<typename Type>
concept ConvertableToJson = requires(Type t, nlohmann::json j) {
  { Type::from_json(j) } -> std::same_as<Type>;
  { t.to_json() } -> std::same_as<nlohmann::json>;
};

template<typename T>
concept DBComparable = requires(T t) {
  { t.hash() } -> std::same_as<std::string>;
  { t.timestamp() } -> std::same_as<uint64_t>;
};

template<typename DataType>
  requires ConvertableToJson<DataType> && DBComparable<DataType>
class DBInternface {

 public:
  size_t size() const {
    size_t counter = 0;
    leveldb::ReadOptions options;
    options.snapshot = m_db->GetSnapshot();
    auto itr_begin = std::unique_ptr<leveldb::Iterator>(m_db->NewIterator(options));

    for (itr_begin->SeekToFirst(); itr_begin->Valid(); itr_begin->Next()) {
      counter++;
    }
    m_db->ReleaseSnapshot(options.snapshot);

    return counter;
  }

  concurrencpp::generator<DataType> data() const {
    leveldb::ReadOptions options;
    options.snapshot = m_db->GetSnapshot();
    auto itr = std::unique_ptr<leveldb::Iterator>(m_db->NewIterator(options));

    for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {

      co_yield from_slice(itr->value());
    }
    m_db->ReleaseSnapshot(options.snapshot);
  }

  std::optional<DataType> get(std::string hash) const {
    leveldb::ReadOptions options;
    options.snapshot = m_db->GetSnapshot();
    auto value_str = std::string{};
    auto status = m_db->Get(options, hash, &value_str);

    if (status.ok() && !status.IsNotFound()) {
      auto vlaue_json = nlohmann::json::parse(value_str);
      return DataType::from_json(vlaue_json);
    }
    m_db->ReleaseSnapshot(options.snapshot);
    return {};
  }

  bool contains(DataType value) {

    return get(value.hash()) != std::nullopt;
  }

  bool contains(std::string hash) {

    return get(hash) != std::nullopt;
  }

  bool put(DataType value) {

    if (!contains(value)) {
      auto string_representation = value.to_json().dump();
      return m_db->Put(m_write_options, value.hash(), string_representation).ok();
    }
    return false;
  }

  bool remove(DataType value) {
    if (contains(value)) {

      return m_db->Delete(m_write_options, value.hash()).ok();
    }
    return false;
  }

  bool remove(std::string hash) {

    if (contains(hash)) {

      return m_db->Delete(m_write_options, hash).ok();
    }
    return false;
  }

  DataType last() {
    leveldb::ReadOptions options;
    options.snapshot = m_db->GetSnapshot();
    auto it = std::unique_ptr<leveldb::Iterator>(m_db->NewIterator(options));
    DataType last_value;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      auto value = from_slice(it->value());
      if (value.timestamp() > last_value.timestamp()) {
        last_value = value;
      }
    }
    m_db->ReleaseSnapshot(options.snapshot);
    return last_value;
  }

  DBInternface() {
    m_db_options.create_if_missing = true;
    m_write_options.sync = true;
  }


 protected:
  bool initialize(std::string path) {
    leveldb::DB *db;
    if (!std::filesystem::exists(path)) {

      if (!std::filesystem::create_directories(path))
        return false;
    }
    auto status = leveldb::DB::Open(m_db_options, path, &db);

    if (status.ok()) {

      m_db = std::unique_ptr<leveldb::DB>(db);
      return true;
    }
    return false;
  }

 private:
  static inline DataType from_slice(const leveldb::Slice &slice) {

    return DataType::from_json(nlohmann::json::parse(slice.ToString()));
  }
  std::unique_ptr<leveldb::DB> m_db;
  leveldb::Options m_db_options;
  leveldb::WriteOptions m_write_options;
};