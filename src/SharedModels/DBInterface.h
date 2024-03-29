#pragma once

#include "Concepts.h"
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


namespace krapi {
  template<typename DataType>
    requires ConvertableToJson<DataType> && DBComparable<DataType> && TaggableConcept<DataType>
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

    std::optional<DataType> get(std::string tag) const {
      leveldb::ReadOptions options;
      options.snapshot = m_db->GetSnapshot();
      auto value_str = std::string{};
      auto status = m_db->Get(options, tag, &value_str);

      if (status.ok() && !status.IsNotFound()) {
        auto vlaue_json = nlohmann::json::parse(value_str);
        return DataType::from_json(vlaue_json);
      }
      m_db->ReleaseSnapshot(options.snapshot);
      return {};
    }

    bool contains(DataType value) {

      return get(value.tag()) != std::nullopt;
    }

    bool contains(std::string tag) {

      return get(tag) != std::nullopt;
    }

    bool put(DataType value) {

      if (!contains(value)) {
        auto string_representation = value.to_json().dump();
        return m_db->Put(m_write_options, value.tag(), string_representation).ok();
      }
      return false;
    }

    bool remove(DataType value) {
      if (contains(value)) {

        return m_db->Delete(m_write_options, value.tag()).ok();
      }
      return false;
    }

    bool remove(std::string tag) {

      if (contains(tag)) {

        return m_db->Delete(m_write_options, tag).ok();
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
}// namespace krapi