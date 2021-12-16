#include "minimum_sleep.h"

#include "tools.h"


namespace {

   auto measure() -> result_unit {
      const auto t0 = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(1ms);
      const auto t1 = std::chrono::high_resolution_clock::now();
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
      return ns;
   }

}

auto minimum_sleep(serialize_type& data, const int n) -> void {
   add_serialization_part(data, measure, n, "minimum_sleep");
}
