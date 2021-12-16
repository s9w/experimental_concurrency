#include "atomic_flag_clear_latency.h"
#include "atomic_flag_test_latency.h"
#include "semaphore_latency.h"
#include "contention_atomic.h"
#include "contention_mutex.h"
#include "json_write.h"
#include "raw_mutex_lock_latency.h"
#include "minimum_sleep.h"
#include "mutex_lock_unlock_latency_st.h"
#include "spinlock_latency.h"
#include "thread_start_cost.h"
#include "thread_start_latency.h"
#include "tools.h"
#include "unique_lock_latency.h"


int main() {
   static_assert(std::is_same_v<std::chrono::high_resolution_clock::duration, std::chrono::nanoseconds>, "No michael no no michael that was so not right");
   constexpr int n = 20'000;

   serialize_type serializations; 

   //{
   //   std::cout << "You can set thread affinity now if you want. Press ENTER to continue\n";
   //   std::cin.get();
   //}

   thread_start_latency(serializations, n);
   thread_start_cost(serializations, n);

   semaphore_latency(serializations, n);
   atomic_flag_test_latency(serializations, n);
   atomic_flag_clear_latency(serializations, n);
   unique_lock_latency(serializations, n);
   mutex_lock_unlock_latency_st(serializations, n);
   raw_mutex_lock_latency(serializations, n);


   spinlock_latency(serializations, n);

   contention_atomic(serializations, n);
   contention_mutex(serializations, n);

   //minimum_sleep(n);

   json_write(serializations, "python/json_out.txt");

   return 0;
}
