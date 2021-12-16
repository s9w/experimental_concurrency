#include "thread_start_cost.h"

#include <thread>

#include "tools.h"


namespace {

   std::atomic<std::chrono::high_resolution_clock::time_point> dummy;

   auto thread_fun() -> void {
      const auto time = std::chrono::high_resolution_clock::now();
      dummy.store(time);
   }

   auto measure() -> result_unit {
      const auto t0 = std::chrono::high_resolution_clock::now();
      std::jthread t(thread_fun);
      const auto t1 = std::chrono::high_resolution_clock::now();

      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
      return ns;
   }
}


auto thread_start_cost(serialize_type& data, const int n) -> void
{
   add_serialization_part(data, measure, n, "thread_start_cost");
}