#include "atomic_flag_latency.h"

#include <mutex>

#include "tools.h"


auto measure_atomic_flag_latency(const int n) -> void {
   std::atomic_flag atomic_flag{};

   const auto setup = [&]() {
      atomic_flag.clear();
   };
   const auto tick_fun = [&]() {
      atomic_flag.test_and_set();
      atomic_flag.notify_one();
   };
   const auto tock_fun = [&]()
   {
      atomic_flag.wait(false);
   };

   measurer m(n, "atomic_flag_latency", setup, tick_fun, tock_fun);
}
