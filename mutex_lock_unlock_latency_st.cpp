#include "mutex_lock_unlock_latency_st.h"

#include <mutex>

#include "tools.h"

namespace {
   easy_atomic<std::chrono::high_resolution_clock::time_point> t1_atomic;

   auto measure() -> result_unit {
      std::mutex mutex; // unlocked initially
      mutex.lock();
      constexpr int n = 10000;

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

auto mutex_lock_unlock_latency_st(const int n) -> void {
   just_do_it(n, "mutex_lock_unlock_latency_st", measure);
}
