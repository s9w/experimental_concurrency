#include "contention_mutex.h"

#include <atomic>
#include <mutex>

#include "tools.h"

namespace
{
   
   std::atomic_flag start_signal;
   int global_int = 0;
   std::mutex mutex;

   std::thread::id last_id;
   int same_ids = 0;
   int different_ids = 0;

   auto thread_fun(std::latch& ready_signal, std::latch& end_signal, const int adds) -> void
   {
      const auto this_id = std::this_thread::get_id();
      ready_signal.count_down();
      ready_signal.wait();
      start_signal.wait(false);

      for (int i = 0; i < adds; ++i) {
         std::scoped_lock lock(mutex);
         global_int += 1;

         if (this_id == last_id)
            ++same_ids;
         else
            ++different_ids;
         last_id = this_id;
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
      start_signal.clear();
      return (t1 - t0).count() / adds;
   }

} // namespace {}


auto contention_mutex(serialize_type& data, const int n) -> void
{
   add_serialization_part(data, []() {return measure(1000); }, n, "contention_mutex");
   std::cout << "same_ratio: " << 100.0 * same_ids / (same_ids + different_ids) << "\n";
   std::cout << "changes: " << different_ids << "\n";
}
