#pragma once

#include <atomic>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <latch>

#include <chrono>
using namespace std::chrono_literals;
using result_unit = typename std::chrono::nanoseconds::rep;

static const int contention_thread_count = static_cast<int>(std::thread::hardware_concurrency())/2 - 1;

// To std::atomic::wait(), you need a neutral or old value. std::optional<T> is great for this,
// but easy to use wrong because of the bitwise comparison. This wraps this correctly.
template<typename T>
struct easy_atomic{
   constexpr static inline auto zero_value = std::optional<T>{};
   std::atomic<std::optional<T>> m_atomic;

   constexpr easy_atomic() noexcept : m_atomic(zero_value){}

   auto wait_for_non_nullopt() const noexcept -> void{
      m_atomic.wait(zero_value);
   }

   [[nodiscard]] auto wait_for_non_nullopt_and_exchange(
      std::memory_order order = std::memory_order_seq_cst
   ) noexcept -> T
   {
      m_atomic.wait(zero_value, order);
      return *m_atomic.exchange(zero_value, order);
   }

   auto store_and_notify_one(
      T new_value,
      std::memory_order order = std::memory_order_seq_cst
   ) noexcept -> void
   {
      m_atomic.store(new_value, order);
      m_atomic.notify_one();
   }

   auto store_and_notify_all(
      T new_value,
      std::memory_order order = std::memory_order_seq_cst
   ) noexcept -> void
   {
      m_atomic.store(new_value, order);
      m_atomic.notify_all();
   }
};

struct console_cursor_disabler{
   console_cursor_disabler() { std::cout << "\x1b[?25l";}
   ~console_cursor_disabler(){ std::cout << "\x1b[?25h";}
};


[[nodiscard]] auto get_percentile(const std::vector<double>& vec, double percentile) -> double;
[[nodiscard]] auto goto_horizontal(int pos) -> std::string;


struct progress_reporter{
   int m_progress = 0;
   int m_max = 0;
   std::optional<std::chrono::high_resolution_clock::time_point> m_last_report{};

   progress_reporter(const int max) : m_max(max) {}
   auto increase() -> void{ ++m_progress; }
   auto get_percent() -> std::optional<std::string>;
};


template<typename fun_type>
auto just_do_it(
   const int n,
   const std::string& description,
   const fun_type& get_measurement
) -> void
{
   console_cursor_disabler no_cursor;
   std::cout << description << ":";
   const int progress_pos = static_cast<int>(description.size()) + 3;
   std::vector<result_unit> runtimes;
   runtimes.reserve(n);
   {
      progress_reporter reporter(n);
      for (int i = 0; i < n; ++i) {
         if (const auto rep = reporter.get_percent(); rep.has_value())
            std::cout << goto_horizontal(progress_pos) << *rep;
         const auto measurement = get_measurement();
         reporter.increase();

         // Skip the first data point
         if(i==0)
            continue;

         runtimes.emplace_back(measurement);
      }
   }

   // Result writing
   std::string filename = "python/";
   filename += description;
   filename += ".txt";

   std::ofstream file_writer(filename);
   for (const result_unit value : runtimes)
      file_writer << std::to_string(value) << "\n";
   std::cout << goto_horizontal(progress_pos) << "done\n";
}
