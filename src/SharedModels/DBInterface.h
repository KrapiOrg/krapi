#pragma once
#include "fmt/format.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "leveldb/iterator.h"
#include "nlohmann/json.hpp"
#include <cstdint>

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

    auto itr_begin =
      std::unique_ptr<leveldb::Iterator>(m_db->NewIterator(m_read_options));

    for (itr_begin->SeekToFirst(); itr_begin->Valid(); itr_begin->Next()) {
      counter++;
    }

    return counter;
  }

  std::vector<DataType> get_all() const {
    std::vector<DataType> data;
    auto itr =
      std::unique_ptr<leveldb::Iterator>(m_db->NewIterator(m_read_options));

    for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {

      data.push_back(from_slice(itr->value()));
    }
    return data;
  }

  std::optional<DataType> get(std::string hash) const {

    auto value_str = std::string{};
    auto status = m_db->Get(m_read_options, hash, &value_str);

    if (status.ok() && !status.IsNotFound()) {
      auto vlaue_json = nlohmann::json::parse(value_str);
      return DataType::from_json(vlaue_json);
    }
    return {};
  }

  bool put(DataType value) {

    auto string_representation = value.to_json().dump();
    return m_db->Put(m_write_options, value.hash(), string_representation).ok();
  }

  bool remove(DataType value) {

    return !m_db->Delete(m_write_options, value.hash()).IsNotFound();
  }

  bool remove(std::string hash) {

    return !m_db->Delete(m_write_options, hash).IsNotFound();
  }

  DataType last() {
    auto it = m_db->NewIterator(m_read_options);
    DataType last_value;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      auto value = from_slice(it->value());
      if (value.timestamp() > last_value.timestamp()) { last_value = value; }
    }
    return last_value;
  }

  DBInternface() {
    m_db_options.create_if_missing = true;
    m_write_options.sync = true;
  }


 protected:
  bool initialize(std::string path) {
    leveldb::DB *db;
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
  leveldb::ReadOptions m_read_options;
  leveldb::WriteOptions m_write_options;
};