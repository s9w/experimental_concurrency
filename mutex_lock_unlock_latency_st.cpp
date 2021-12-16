#include "mutex_lock_unlock_latency_st.h"

#include <mutex>

#include "tools.h"

namespace {

   auto measure() -> result_unit {
      std::mutex mutex; // unlocked initially
      mutex.lock();
      constexpr int n = 10'000;

      const auto t0 = std::chrono::high_resolution_clock::now();
      for(int i=0; i<n; ++i){
         mutex.unlock();
         mutex.lock();
      }
      
      const auto t1 = std::chrono::high_resolution_clock::now();
      mutex.unlock();
      return (t1 - t0).count() / n;
   }

}

auto mutex_lock_unlock_latency_st(serialize_type& data, const int n) -> void {
   add_serialization_part(data, measure, n, "mutex_lock_unlock_latency_st");
}
