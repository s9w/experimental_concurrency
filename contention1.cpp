#include "contention0.h"

#include <atomic>
#include <latch>
#include <mutex>

#include "tools.h"

namespace
{
   
   std::atomic_flag start_signal;
   int global_int = 0;
   std::mutex mutex;

   auto thread_fun(std::latch& ready_signal, std::latch& end_signal, const int adds) -> void
   {
      ready_signal.count_down();
      ready_signal.wait();
      start_signal.wait(false);

      for (int i = 0; i < adds; ++i) {
         mutex.lock();
         global_int += 1;
         mutex.unlock();
      }

      end_signal.count_down();
   }


   auto measure(const int adds) -> result_unit
   {
      global_int = 0;
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

} // namespace {}


auto contention1(const int n) -> void
{
   just_do_it(n, "contention_mutex", [](){return measure( 1000); });
}
