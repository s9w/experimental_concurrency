#include "condition_variable_latency.h"

#include <thread>
#include <condition_variable>

#include "tools.h"

namespace {
   std::condition_variable cv;
   std::mutex cv_m;

   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;


   auto acquire_and_set_t1() {
      std::unique_lock<std::mutex> lk(cv_m);
      cv.wait(lk);
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
   }

   auto measure() -> double {
      std::jthread j(acquire_and_set_t1);
      std::this_thread::sleep_for(max_threadup_spinup_time); // To make sure thread is spun up

      // Signal thread and time
      const auto t0 = std::chrono::high_resolution_clock::now();
      cv.notify_one();
      std::this_thread::sleep_for(max_thread_write_time);

      const auto opt = t1.load();
      if (opt.has_value() == false)
         std::terminate();
      const double ns = std::chrono::duration_cast<dbl_ns>(*opt - t0).count();

      t1.store(std::nullopt);
      return ns;
   }

}

auto measure_conditionvar_latency(int n) -> void {
   std::vector<double> runtimes;
   runtimes.reserve(n);
   for (int i = 0; i < n; ++i) {
      if (i % 100 == 0)
         std::cout << oof::hposition(0) << 100 * i / n << "%";
      runtimes.emplace_back(measure());
   }
   std::cout << "\n";
   report(runtimes, "condition_variable_latency");
}
