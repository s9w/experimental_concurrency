#include "atomic_flag_test_latency.h"

#include <atomic>

#include "tools.h"

namespace {
   std::atomic_flag ready_signal;
   curry::easy_atomic<std::chrono::high_resolution_clock::time_point> t1_atomic;

   std::atomic_flag atomic_flag{}; // false/clear init
   

   auto thread_fun() -> void {
      ready_signal.test_and_set();
      ready_signal.notify_one();

      atomic_flag.wait(false);
      const auto time = std::chrono::high_resolution_clock::now();
      t1_atomic.store_and_notify_one(time);
      atomic_flag.clear();
   }

   auto measure() -> curry::result_unit {
      std::jthread j(thread_fun);
      ready_signal.wait(false);

      const auto t0 = std::chrono::high_resolution_clock::now();
      atomic_flag.test_and_set();
      atomic_flag.notify_one();

      ready_signal.clear();
      return (t1_atomic.wait_for_non_nullopt_and_exchange() - t0).count();
   }

}


auto curry::atomic_flag_test_latency(serialize_type& data, const int n) -> void
{
   add_payload(data, measure, n, "atomic_flag_test_latency");
}
