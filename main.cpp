#include "atomic_flag_clear_latency.h"
#include "atomic_flag_test_latency.h"
#include "semaphore_latency.h"
#include "contention_atomic_flag.h"
#include "contention_atomic_add.h"
#include "contention_atomic_large.h"
#include "contention_mutex.h"
#include "json_write.h"
#include "map_threads.h"
#include "minimum_sleep.h"
#include "mutex_lock_unlock_latency_st.h"
#include "spinlock_latency.h"
#include "thread_start_cost.h"
#include "thread_start_latency.h"
#include "tools.h"
#include "scoped_lock_latency.h"

#define OOF_IMPL
#include "oof.h"

int main() {
   using namespace excon;

   static_assert(std::is_same_v<std::chrono::high_resolution_clock::duration, std::chrono::nanoseconds>, "No michael no no michael that was so not right");
   constexpr int n = 50'000;

   serialize_type serializations; 

   thread_start_latency(serializations, n);
   thread_start_cost(serializations, n);

   spinlock_latency(serializations, n);
   semaphore_latency(serializations, n);
   atomic_flag_test_latency(serializations, n);
   scoped_lock_latency(serializations, n);




   contention_atomic_flag(serializations, n);
   contention_atomic_add(serializations, n);
   contention_mutex(serializations, n);

  // map_threads(serializations);

   //mutex_lock_unlock_latency_st(serializations, n);
   //minimum_sleep(n);

   json_write(serializations, "analysis/json_out.txt");

   return 0;
}
