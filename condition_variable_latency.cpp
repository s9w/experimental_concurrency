#include "condition_variable_latency.h"

#include <mutex>

#include "tools.h"


namespace {
   std::atomic_flag ready_signal;

   std::condition_variable condition_var;
   std::mutex mutex;
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto thread_fun() -> void {
      ready_signal.test_and_set();
      ready_signal.notify_one();

      std::unique_lock<std::mutex> lk(mutex);
      condition_var.wait(lk);
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
      t1.notify_one();
   }

   auto measure() -> result_unit {
      std::jthread j(thread_fun);
      ready_signal.wait(false);

      const auto t0 = std::chrono::high_resolution_clock::now();
      condition_var.notify_one();

      t1.wait(std::nullopt);
      const auto loaded = t1.exchange(std::nullopt);
      if (loaded.has_value() == false)
         std::terminate();
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(*loaded - t0).count();
      ready_signal.clear();
      return ns;
   }

}

auto conditionvar_latency(const int n) -> void
{
   just_do_it(n, "condition_variable_latency", measure);
}
