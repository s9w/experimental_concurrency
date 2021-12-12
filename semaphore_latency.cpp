#include "semaphore_latency.h"

#include <semaphore>
#include <thread>
#include <iostream>

#include "tools.h"

namespace {
   std::binary_semaphore semaphore{ 0 };
   struct thread_result
   {
      std::chrono::high_resolution_clock::time_point t1{};
      int processor_number{};
   };
   std::atomic_bool should_end = false;
   std::atomic_bool armed = false;
   std::atomic<std::optional<thread_result>> t1;


   auto thread_fun() -> void
   {
      while(should_end.load() == false)
      {
         armed.wait(false);

         semaphore.acquire();
         const auto time = std::chrono::high_resolution_clock::now();

         t1.store(thread_result{ time, 42 });
         armed.store(false);
      }
      std::cout << "ended\n";
   }

   auto measure() -> paired_time {
      armed.store(true);
      armed.notify_one();
      std::this_thread::sleep_for(max_thread_write_time);

      const auto t0 = std::chrono::high_resolution_clock::now();
      semaphore.release();
      std::this_thread::sleep_for(max_thread_write_time);

      
      if (t1.load().has_value() == false)
         std::terminate();

      const auto loaded = t1.load();
      const double ns = std::chrono::duration_cast<dbl_ns>(loaded->t1 - t0).count();

      t1.store(std::nullopt);
      const auto processor_number = static_cast<int>(GetCurrentProcessorNumber());
      return paired_time{ processor_number, loaded->processor_number, ns };
   }

}

auto measure_semaphore_latency(const int n) -> void {
   std::jthread j(thread_fun);
   std::this_thread::sleep_for(max_threadup_spinup_time);

   std::vector<paired_time> runtimes;
   runtimes.reserve(n);
   for (int i = 0; i < n; ++i) {
      if (i % 100 == 0)
         std::cout << 100 * i / n << "% ";
      if(i==n-1)
         should_end.store(true);
      runtimes.emplace_back(measure());
   }
   std::cout << "\n";
   report(runtimes, "semaphore_latency");
   
}
