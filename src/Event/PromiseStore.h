//
// Created by mythi on 31/12/22.
//

#pragma once

#include "Event.h"
#include "concurrencpp/concurrencpp.h"

namespace krapi {
  template<typename ResultType, typename TagType = std::string>
  class PromiseStore {
    mutable std::recursive_mutex m_mutex;
    std::unordered_map<TagType, concurrencpp::result_promise<ResultType>>
      m_promises;

   public:
    concurrencpp::shared_result<ResultType> add(TagType tag) {

      auto promise = concurrencpp::result_promise<ResultType>();
      auto result = promise.get_result();

      {
        std::lock_guard l(m_mutex);
        m_promises.emplace(tag, std::move(promise));
      }
      return result;
    }

    bool contains(TagType tag) const {

      std::lock_guard l(m_mutex);
      return m_promises.contains(tag);
    }

    bool set_result(std::string tag, ResultType result) {

      std::lock_guard l(m_mutex);
      if (m_promises.contains(tag)) {

        m_promises.find(tag)->second.set_result(result);
        m_promises.erase(tag);
        return false;
      }
      return true;
    }
  };
}// namespace krapi