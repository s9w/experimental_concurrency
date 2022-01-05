#include "thread_start_latency.h"

#include <thread>

#include "tools.h"

namespace {

   excon::easy_atomic<std::chrono::high_resolution_clock::time_point> t1_atomic;

   auto thread_fun() -> void {
      const auto time = std::chrono::high_resolution_clock::now();
      t1_atomic.store_and_notify_one(time);
   }

   auto measure() -> excon::result_unit {
      const auto t0 = std::chrono::high_resolution_clock::now();
      {
         std::jthread t(thread_fun);
      }

      return (t1_atomic.wait_for_non_nullopt_and_exchange() - t0).count();
   }
}


auto excon::thread_start_latency(serialize_type& data, const int n) -> void
{
   add_payload(data, measure, n, "thread_start_latency");
}
