#include "mutex_lock_latency.h"

#include <mutex>

#include "tools.h"

namespace {
   std::mutex mutex;
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto thread_fun() {
      mutex.lock();
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
   }

   auto measure() -> double {
      mutex.lock();

      std::jthread j(thread_fun);
      std::this_thread::sleep_for(max_threadup_spinup_time);

      const auto t0 = std::chrono::high_resolution_clock::now();
      mutex.unlock();
      std::this_thread::sleep_for(max_thread_write_time);
      const auto opt = t1.load();
      if (opt.has_value() == false)
         std::terminate();
      const double ns = std::chrono::duration_cast<dbl_ns>(*opt - t0).count();

      mutex.unlock();
      t1.store(std::nullopt);

      return ns;
   }

}

auto mutex_lock_latency(const int n) -> void {
   std::vector<double> runtimes;
   runtimes.reserve(n);
   for (int i = 0; i < n; ++i) {
      if (i % 100 == 0)
         std::cout << oof::hposition(0) << 100 * i / n << "%";
      runtimes.emplace_back(measure());
   }
   std::cout << "\n";
   report(runtimes, "mutex_lock_latency");
}
