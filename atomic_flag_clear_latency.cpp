#include "atomic_flag_clear_latency.h"

#include "tools.h"

namespace {
   std::atomic_flag ready_signal;
   easy_atomic<std::chrono::high_resolution_clock::time_point> t1_atomic;

   std::atomic_flag atomic_flag{}; // false/clear init
   

   auto thread_fun() -> void {
      ready_signal.test_and_set();
      ready_signal.notify_one();

      atomic_flag.wait(true);
      const auto time = std::chrono::high_resolution_clock::now();
      t1_atomic.store_and_notify_one(time);
      atomic_flag.test_and_set();
   }

   auto measure() -> result_unit {
      std::jthread j(thread_fun);
      ready_signal.wait(false);

      const auto t0 = std::chrono::high_resolution_clock::now();
      atomic_flag.clear();
      atomic_flag.notify_one();

      ready_signal.clear();
      return (t1_atomic.wait_for_non_nullopt_and_exchange() - t0).count();
   }

} // namespace {}


auto atomic_flag_clear_latency(serialize_type& data, const int n) -> void
{
   atomic_flag.test_and_set();
   add_payload(data, measure, n, "atomic_flag_clear_latency");
}
