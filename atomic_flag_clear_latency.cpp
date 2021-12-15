#include "atomic_flag_test_latency.h"

#include <mutex>

#include "tools.h"

namespace {
   std::atomic_flag ready_signal;

   std::atomic_flag atomic_flag{}; // false/clear init
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto thread_fun() -> void {
      ready_signal.test_and_set();
      ready_signal.notify_one();

      atomic_flag.wait(true);
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
      t1.notify_one();
      atomic_flag.test_and_set();
   }

   auto measure() -> result_unit {
      std::jthread j(thread_fun);
      ready_signal.wait(false);

      const auto t0 = std::chrono::high_resolution_clock::now();
      atomic_flag.clear();
      atomic_flag.notify_one();

      t1.wait(std::nullopt);
      const auto loaded = t1.exchange(std::nullopt);
      if (loaded.has_value() == false)
         std::terminate();
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(*loaded - t0).count();
      ready_signal.clear();
      return ns;
   }

}


auto atomic_flag_clear_latency(const int n) -> void
{
   atomic_flag.test_and_set();
   just_do_it(n, "atomic_flag_clear_latency", measure);
}
