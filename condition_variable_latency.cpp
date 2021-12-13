#include "condition_variable_latency.h"

#include <mutex>

#include "tools.h"


namespace {
   std::condition_variable condition_var;
   std::mutex mutex;
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto thread_fun() -> void {
      std::unique_lock<std::mutex> lk(mutex);
      condition_var.wait(lk);
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
      t1.notify_one();
   }

   auto measure() -> result_unit {
      std::jthread j(thread_fun);
      std::this_thread::sleep_for(max_threadup_spinup_time);

      const auto t0 = std::chrono::high_resolution_clock::now();
      condition_var.notify_one();

      std::this_thread::sleep_for(max_latency);
      const auto loaded = t1.exchange(std::nullopt);
      if (loaded.has_value() == false)
         std::terminate();
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(*loaded - t0).count();
      return ns;
   }

}

auto conditionvar_latency(const int n) -> void
{
   just_do_it(n, "condition_variable_latency", measure);
}
