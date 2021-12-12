#include "semaphore_latency.h"

#include <semaphore>

#include "tools.h"


auto measure_semaphore_latency(const int n) -> void {
   std::binary_semaphore semaphore{ 0 };

   const auto tick_fun = [&]() {semaphore.release(); };
   const auto tock_fun = [&]() {semaphore.acquire(); };
   
   measurer m;
   m.start(n, tick_fun, tock_fun);
}
