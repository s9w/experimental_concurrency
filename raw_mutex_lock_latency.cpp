#include "raw_mutex_lock_latency.h"

#include <mutex>

#include "tools.h"

namespace {
   std::atomic_flag ready_signal;
   easy_atomic<std::chrono::high_resolution_clock::time_point> t1_atomic;

   std::mutex mutex; // unlocked initially

   auto thread_fun() -> void {
      ready_signal.test_and_set();
      ready_signal.notify_one();

      mutex.lock();
      const auto time = std::chrono::high_resolution_clock::now();
      t1_atomic.store_and_notify_one(time);
      mutex.unlock(); // Same thread needs to unlock
   }

   auto measure() -> result_unit {
      mutex.lock(); // mutex is unlocked initially but want to measure unlock()-lock()

      std::jthread j(thread_fun);
      ready_signal.wait(false);

      const auto t0 = std::chrono::high_resolution_clock::now();
      mutex.unlock();

      ready_signal.clear();
      return (t1_atomic.wait_for_non_nullopt_and_exchange() - t0).count();
   }

}

auto raw_mutex_lock_latency(const int n) -> void {
   just_do_it(n, "raw_mutex_lock_latency", measure);
}
