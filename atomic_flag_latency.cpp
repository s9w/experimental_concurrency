#include "semaphore_latency.h"

#include <semaphore>
#include <thread>
#include <iostream>

#include "tools.h"

namespace {
   std::atomic_flag atomic_flag{  };
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;


   auto acquire_and_set_t1() {
      atomic_flag.wait(false);
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
   }

   auto measure() -> double {
      atomic_flag.clear();
      // startup thread
      std::jthread j(acquire_and_set_t1);
      std::this_thread::sleep_for(max_threadup_spinup_time);

      // Signal thread and time
      const auto t0 = std::chrono::high_resolution_clock::now();
      atomic_flag.test_and_set();
      atomic_flag.notify_one();
      std::this_thread::sleep_for(max_thread_write_time);

      const auto tp = t1.load();
      if (tp.has_value() == false)
         std::terminate();
      const double ns = std::chrono::duration_cast<dbl_ns>(*tp - t0).count();
      
      t1.store(std::nullopt);
      return ns;
   }

}

auto measure_atomic_flag_latency(const int n) -> void {

   std::vector<double> runtimes;
   runtimes.reserve(n);
   for (int i = 0; i < n; ++i) {
      if (i % 100 == 0)
         std::cout << oof::hposition(0) << 100 * i / n << "%";
      runtimes.emplace_back(measure());
   }
   std::cout << "\n";
   report(runtimes, "atomic_flag_latency");
}
