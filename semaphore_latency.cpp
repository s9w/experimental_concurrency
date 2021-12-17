#include "semaphore_latency.h"

#include <semaphore>

#include "tools.h"

namespace {
   std::atomic_flag ready_signal;
   curry::easy_atomic<std::chrono::high_resolution_clock::time_point> t1_atomic;

   std::binary_semaphore semaphore{ 0 };

   auto thread_fun() -> void {
      ready_signal.test_and_set();
      ready_signal.notify_one();

      semaphore.acquire();
      const auto time = std::chrono::high_resolution_clock::now();
      t1_atomic.store_and_notify_one(time);
   }

   auto measure() -> curry::result_unit {
      std::jthread j(thread_fun);
      ready_signal.wait(false);

      const auto t0 = std::chrono::high_resolution_clock::now();
      semaphore.release();

      ready_signal.clear();
      return (t1_atomic.wait_for_non_nullopt_and_exchange() - t0).count();
   }

}

auto curry::semaphore_latency(serialize_type& data, const int n) -> void
{
   add_payload(data, measure, n, "semaphore_latency");
}
