#include "contention_atomic_large.h"

#include <atomic>

#include "tools.h"

namespace
{
   using namespace excon;

   struct large_int{
      int value;
      uint8_t filler[16 - 4];
   };
   static_assert(sizeof(large_int) == 16);

   

   std::atomic_flag start_signal;
   std::atomic<large_int> atomic;

   auto thread_fun(std::latch& ready_signal, std::latch& end_signal, const int adds) -> void
   {
      ready_signal.count_down();
      ready_signal.wait();
      start_signal.wait(false);

      for(int i=0; i<adds; ++i)
      {
         atomic.wait(large_int{-1});
         atomic.store(large_int{ atomic.load().value + 1 });
         atomic.notify_one();
      }

      end_signal.count_down();
   }

   auto measure(const int adds) -> result_unit
   {
      atomic.store(large_int{0});
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
      start_signal.clear();
      return (t1 - t0).count() / adds;
   }
}


auto excon::contention_atomic_large(serialize_type& data, const int n) -> void
{
   add_payload(data, []() {return measure(1000); }, n, "contention_atomic_large");
}

