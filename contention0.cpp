#include "contention0.h"

#include <atomic>

#include "tools.h"

namespace
{
   std::atomic_flag start_signal;
   std::atomic<int> atomic;

   auto thread_fun(std::latch& ready_signal, std::latch& end_signal, const int adds) -> void
   {
      ready_signal.count_down();
      ready_signal.wait();
      start_signal.wait(false);

      for(int i=0; i<adds; ++i)
         atomic.fetch_add(1);

      end_signal.count_down();
   }

   auto measure(const int adds) -> result_unit
   {
      atomic.store(0);
      std::vector<std::jthread> threads;
      threads.reserve(contention_thread_count);
      std::latch ready_signal(contention_thread_count);
      std::latch end_signal(contention_thread_count);
      for (int i = 0; i < contention_thread_count; ++i)
         threads.emplace_back(thread_fun, std::ref(ready_signal), std::ref(end_signal), adds);
      
      ready_signal.wait();

      const auto t0 = std::chrono::high_resolution_clock::now();
      start_signal.test_and_set();
      start_signal.notify_all();
      end_signal.wait();
      const auto t1 = std::chrono::high_resolution_clock::now();
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
      start_signal.clear();
      return ns;
   }
}


auto contention0(const int n) -> void
{
   just_do_it(n, "contention_atomic", []() {return measure( 1000); });
}

