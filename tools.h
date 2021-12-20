#pragma once

#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <latch>
#include <unordered_map>

#include <chrono>

namespace curry {
   using result_unit = typename std::chrono::nanoseconds::rep;

   static const int contention_thread_count = static_cast<int>(std::thread::hardware_concurrency()) / 2 - 1;

   // To std::atomic::wait(), you need a neutral or old value. std::optional<T> is great for this,
   // but easy to use wrong because of the bitwise comparison. This wraps this correctly.
   template<typename T>
   struct easy_atomic {
      constexpr static auto x = sizeof(T);
      constexpr static inline auto zero_value = std::optional<T>{};
      std::atomic<std::optional<T>> m_atomic;
      //static_assert(sizeof(std::optional<T>)<=8);

      constexpr easy_atomic() noexcept : m_atomic(zero_value) {}
      auto wait_for_non_nullopt() const noexcept -> void;
      [[nodiscard]] auto wait_for_non_nullopt_and_exchange(std::memory_order order = std::memory_order_seq_cst) noexcept -> T;
      auto store_and_notify_one(T new_value, std::memory_order order = std::memory_order_seq_cst) noexcept -> void;
      auto store_and_notify_all(T new_value, std::memory_order order = std::memory_order_seq_cst) noexcept -> void;
   };


   struct console_cursor_disabler {
      console_cursor_disabler() { std::cout << "\x1b[?25l"; }
      ~console_cursor_disabler() { std::cout << "\x1b[?25h"; }
   };


   [[nodiscard]] auto get_horizontal_pos_str(int pos) -> std::string;
   auto set_thread_affinity(const std::vector<int>& cpus) -> void;
   [[nodiscard]] auto get_logical_processor_number() -> int;
   [[nodiscard]] auto get_50th_percentile(const std::vector<result_unit>& vec) -> result_unit;


   struct progress_reporter {
      int m_progress = 0;
      int m_max = 0;
      std::optional<std::chrono::high_resolution_clock::time_point> m_last_report_time{};
      int m_progress_pos{};
      console_cursor_disabler no_cursor;

      explicit progress_reporter(const std::string& description, int max);
      ~progress_reporter();
      auto report() -> void;
      [[nodiscard]] auto get_percent()->std::optional<std::string>;
   };


   using serialize_type = std::unordered_map<std::string, std::vector<result_unit>>;

   template<typename fun_type>
   auto add_payload(
      serialize_type& data,
      const fun_type& fun,
      const int n,
      const std::string& description
   ) -> void
   {
      std::vector<result_unit> payload;
      payload.reserve(n);
      progress_reporter reporter(description, n);
      for (int i = 0; i < n; ++i) {
         reporter.report();
         payload.emplace_back(fun());
      }

      data.emplace(description, std::move(payload));
   }

}
