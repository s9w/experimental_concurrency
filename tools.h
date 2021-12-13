#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <chrono>
using namespace std::chrono_literals;
using result_unit = typename std::chrono::nanoseconds::rep;


inline auto max_threadup_spinup_time = 5.0ms;
constexpr auto max_latency = 50us;


inline auto write_results(
   const std::vector<result_unit>& vec,
   const char* description
) -> void
{
   std::string filename = "python/";
   filename += description;
   filename += ".txt";

   std::ofstream file_writer(filename);
   for (const result_unit value : vec)
      file_writer << std::to_string(value) << "\n";
   std::cout << description << " done\n";
}


struct console_cursor_disabler{
   console_cursor_disabler() { std::cout << "\x1b[?25l";}
   ~console_cursor_disabler(){ std::cout << "\x1b[?25h";}
};


struct measurer{
   std::atomic_bool m_should_end = false;
   std::atomic_bool m_armed = false;
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> m_t1;

   template<typename tock_fun_type>
   auto thread_fun(
      const tock_fun_type& tock_fun
   ) -> void
   {
      while (true)
      {
         // Wait until the measurement is "armed"
         m_armed.wait(false);

         // Do whatever is being measured on the spawned thread. Typically
         // semaphore.acquire(), mutex.lock(), condition_variable.wait(), atomic_flag.wait()
         tock_fun();

         // Capture the timestamp and store it in the result variable
         const auto time = std::chrono::high_resolution_clock::now();
         m_t1.store(time);

         // Disarm this thread until the Bookkeeping is done and this thread is re-armed again
         m_armed.store(false);

         // If enough measurements are done, this signal is being switched to true
         if (m_should_end.load() == true)
            return;
      }
   }


   template<typename setup_fun_type, typename tick_fun_type>
   [[nodiscard]] auto generic_measure(
      const setup_fun_type& setup_fun,
      const tick_fun_type& tick_fun
   ) -> result_unit
   {
      setup_fun();

      // Arm the thread so it runs to it's tock() function
      m_armed.store(true);
      m_armed.notify_one();

      // Make sure the message went through
      std::this_thread::sleep_for(max_latency);

      // Start the measurement
      const auto t0 = std::chrono::high_resolution_clock::now();
      tick_fun();

      // Wait until the tock function is guaranteed to have finished
      std::this_thread::sleep_for(max_latency);

      // Retrieve the stored timepoint
      const auto loaded_t1 = m_t1.load();
      if (loaded_t1.has_value() == false) {
         std::cout << "max_latency probably wasn't high enough\n";
         std::terminate();
      }

      // Calculate result and reset things
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(*loaded_t1 - t0).count();
      m_t1.store(std::nullopt);
      return ns;
   }

   template<typename setup_fun_type, typename tick_fun_type, typename tock_fun_type>
   measurer(
      const int n,
      const char* description,
      const setup_fun_type& setup_fun,
      const tick_fun_type& tick_fun,
      const tock_fun_type& tock_fun
   )
   {
      std::jthread thread([&]() {thread_fun(tock_fun); });

      // Make sure thread is started
      std::this_thread::sleep_for(max_threadup_spinup_time);

      std::vector<result_unit> runtimes;
      runtimes.reserve(n);
      {
         console_cursor_disabler no_cursor;
         for (int i = 0; i < n; ++i) {
            if (i % 100 == 0) {
               const int percentage = 100 * i / n;
               std::cout << "\x1b[0G" << percentage << "%    ";
            }
            if (i == n - 1)
               m_should_end.store(true);
            runtimes.emplace_back([&]() {return generic_measure(setup_fun, tick_fun); }());
         }
      }
      std::cout << "\n";
      write_results(runtimes, description);
   }

};



//inline auto write_results(
//   const std::vector<paired_time>& vec,
//   const char* description
//) -> void
//{
//   std::cout << "written " << description << "\n";
//   std::string filename = "python/";
//   filename += description;
//   filename += ".txt";
//
//   std::ofstream filestream(filename);
//   for (const paired_time& value : vec)
//      filestream << std::to_string(value.ns) << " " << std::to_string(value.number0) << " " << std::to_string(value.number1) << "\n";
//}
//
//inline auto write_results(
//   const std::vector<double>& vec,
//   const char* description
//) -> void
//{
//   std::cout << "written " << description << "\n";
//   std::string filename = "python/";
//   filename += description;
//   filename += ".txt";
//   
//   std::ofstream filestream(filename);
//   for (const double value : vec)
//      filestream << std::to_string(value) << "\n";
//}
//
//inline auto write_results(
//   const std::vector<std::pair<double, double>>& vec,
//   const char* description
//) -> void
//{
//   std::cout << "written " << description << "\n";
//   std::string filename = "python/";
//   filename += description;
//   filename += ".txt";
//
//   std::ofstream filestream(filename);
//   for (const auto& [row0, row1]: vec)
//      filestream << std::to_string(row0) << " " << row1 << "\n";
//}
