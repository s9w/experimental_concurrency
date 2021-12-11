#include "thread_start.h"

#include <thread>

#include "tools.h"

namespace {

   struct times{
      double hosting_ns{};
      double worker_ns{};
   };

   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto thread_fun() -> void {
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
   }

   auto measure() -> times {
      times result;
      const auto t0 = std::chrono::high_resolution_clock::now();
      std::jthread t(thread_fun);
      const auto t1_main = std::chrono::high_resolution_clock::now();
      result.hosting_ns = std::chrono::duration_cast<dbl_ns>(t1_main - t0).count();

      std::this_thread::sleep_for(max_threadup_spinup_time);
      if (t1.load().has_value() == false)
         std::terminate();
      const double ns = std::chrono::duration_cast<dbl_ns>(*t1.load() - t0).count();

      t1.store(std::nullopt);
      return result;
   }
}


auto measure_thread_start(const int n) -> void
{
   std::vector<double> ns_hosting;
   std::vector<double> ns_worker;
   ns_hosting.reserve(n);
   ns_worker.reserve(n);
   for (int i = 0; i < n; ++i) {
      if (i % 1000 == 0)
         std::cout << 100 * i / n << "% ";
      const auto& [hosting, worker] = measure();
      ns_hosting.emplace_back(hosting);
      ns_worker.emplace_back(worker);
   }
   std::cout << "\n";
   report(ns_hosting, "thread_start_hosting");
   report(ns_worker, "thread_start_worker");
}