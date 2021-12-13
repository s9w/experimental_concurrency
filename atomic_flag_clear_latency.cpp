#include "atomic_flag_test_latency.h"

#include <mutex>

#include "tools.h"

namespace {
   std::atomic_flag atomic_flag{}; // false/clear init
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto thread_fun() -> void {
      atomic_flag.wait(true);
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
      t1.notify_one();
      atomic_flag.test_and_set();
   }

   auto measure() -> result_unit {
      std::jthread j(thread_fun);
      std::this_thread::sleep_for(max_threadup_spinup_time);

      const auto t0 = std::chrono::high_resolution_clock::now();
      atomic_flag.clear();
      atomic_flag.notify_one();

      std::this_thread::sleep_for(max_latency);
      const auto loaded = t1.exchange(std::nullopt);
      if (loaded.has_value() == false)
         std::terminate();
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(*loaded - t0).count();
      return ns;
   }

}


auto atomic_flag_clear_latency(const int n) -> void
{
   atomic_flag.test_and_set();
   just_do_it(n, "atomic_flag_clear_latency", measure);
}
