#include "atomic_flag_clear_latency.h"
#include "atomic_flag_test_latency.h"
#include "semaphore_latency.h"
#include "contention_atomic.h"
#include "contention_mutex.h"
#include "raw_mutex_lock_latency.h"
#include "minimum_sleep.h"
#include "mutex_lock_unlock_latency_st.h"
#include "spinlock_latency.h"
#include "thread_start_cost.h"
#include "thread_start_latency.h"
#include "tools.h"
#include "unique_lock_latency.h"

int main() {
   static_assert(std::is_same_v<std::chrono::high_resolution_clock::duration, std::chrono::nanoseconds>, "sorry but I assumed this. sue me");
   constexpr int n = 5000;

   //std::this_thread::sleep_for(15s);
   //std::cout << "go\n";

   thread_start_latency(n);
   thread_start_cost(n);

   semaphore_latency(n);
   atomic_flag_test_latency(n);
   atomic_flag_clear_latency(n);
   unique_lock_latency(n);
   mutex_lock_unlock_latency_st(n);
   raw_mutex_lock_latency(n);
   

   spinlock_latency(n);

   contention_atomic(n);
   contention_mutex(n);

   //minimum_sleep(n);



   return 0;
}
