#include "spinlock_latency.h"

#include <semaphore>
#include <thread>

#include "tools.h"


namespace {
   class atomic_flag_spinlock {
      std::atomic_flag m_lock;
   public:
      void lock() {
         while (m_lock.test(std::memory_order_relaxed))
            ;
         while (m_lock.test_and_set(std::memory_order_acquire))
            ;
      }
      void unlock() {
         m_lock.clear(std::memory_order_release);
         m_lock.notify_one();
      }
   };

   struct tas_lock {
      std::atomic<bool> lock_ = { false };

      void lock() {
         for (;;) {
            if (!lock_.exchange(true, std::memory_order_acquire)) {
               break;
            }
            while (lock_.load(std::memory_order_relaxed));
         }
      }

      void unlock() {
         lock_.store(false, std::memory_order_release);
         lock_.notify_one();
      }
   };

   std::atomic_flag ready_signal;
   easy_atomic<std::chrono::high_resolution_clock::time_point> t1_atomic;

   atomic_flag_spinlock spinlock;

   auto thread_fun() -> void {
      ready_signal.test_and_set();
      ready_signal.notify_one();

      spinlock.lock();
      const auto time = std::chrono::high_resolution_clock::now();
      t1_atomic.store_and_notify_one(time);
   }

   auto measure() -> result_unit {
      std::jthread j(thread_fun);
      ready_signal.wait(false);

      const auto t0 = std::chrono::high_resolution_clock::now();
      spinlock.unlock();

      ready_signal.clear();
      return (t1_atomic.wait_for_non_nullopt_and_exchange() - t0).count();
   }

}

auto spinlock_latency(serialize_type& data, const int n) -> void
{
   spinlock.lock();
   add_payload(data, measure, n, "spinlock_latency");
}
