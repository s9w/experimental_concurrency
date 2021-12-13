#include "thread_start_latency.h"

#include <thread>

#include "tools.h"

namespace {

   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto thread_fun() -> void {
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
   }

   auto measure() -> result_unit {
      const auto t0 = std::chrono::high_resolution_clock::now();
      std::jthread t(thread_fun);

      std::this_thread::sleep_for(max_threadup_spinup_time);
      const auto loaded = t1.exchange(std::nullopt);
      if (loaded.has_value() == false)
         std::terminate();
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(*loaded - t0).count();
      return ns;
   }
}


auto thread_start_latency(const int n) -> void
{
   just_do_it(n, "thread_start_latency", measure);
}