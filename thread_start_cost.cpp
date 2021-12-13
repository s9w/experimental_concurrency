#include "thread_start_cost.h"

#include <thread>

#include "tools.h"

//namespace {
//
//   struct times{
//      double hosting_ns{};
//      double worker_ns{};
//   };
//
//   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;
//
//   auto thread_fun() -> void {
//      const auto time = std::chrono::high_resolution_clock::now();
//      t1.store(time);
//   }
//
//   auto measure() -> times {
//      times result;
//      const auto t0 = std::chrono::high_resolution_clock::now();
//      std::jthread t(thread_fun);
//      const auto t1_main = std::chrono::high_resolution_clock::now();
//      result.hosting_ns = std::chrono::duration_cast<dbl_ns>(t1_main - t0).count();
//
//      std::this_thread::sleep_for(max_threadup_spinup_time);
//      if (t1.load().has_value() == false)
//         std::terminate();
//      result.worker_ns = std::chrono::duration_cast<dbl_ns>(*t1.load() - t0).count();
//
//      t1.store(std::nullopt);
//      return result;
//   }
//}

namespace {

   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> dummy;

   auto thread_fun() -> void {
      const auto time = std::chrono::high_resolution_clock::now();
      dummy.store(time);
   }

   auto measure() -> result_unit {
      const auto t0 = std::chrono::high_resolution_clock::now();
      std::jthread t(thread_fun);
      const auto t1 = std::chrono::high_resolution_clock::now();

      std::this_thread::sleep_for(max_threadup_spinup_time);
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
      return ns;
   }
}


auto thread_start_cost(const int n) -> void
{
   just_do_it(n, "thread_start_cost", measure);
}