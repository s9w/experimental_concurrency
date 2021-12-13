#include "condition_variable_latency.h"

#include <mutex>

#include "tools.h"


auto measure_conditionvar_latency(const int n) -> void {
   std::condition_variable condition_var;
   std::mutex mutex;

   const auto setup = [&]() {};
   const auto tick_fun = [&]() {condition_var.notify_one();; };
   const auto tock_fun = [&]()
   {
      std::unique_lock<std::mutex> lk(mutex);
      condition_var.wait(lk);
   };

   measurer m(n, "condition_variable_latency", setup, tick_fun, tock_fun);
}
