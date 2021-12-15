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

   atomic_flag_spinlock spinlock;
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto thread_fun() -> void {
      ready_signal.test_and_set();
      ready_signal.notify_one();

      spinlock.lock();
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
      t1.notify_one();
   }

   auto measure() -> result_unit {
      std::jthread j(thread_fun);
      ready_signal.wait(false);

      const auto t0 = std::chrono::high_resolution_clock::now();
      spinlock.unlock();
      t1.wait(std::nullopt);
      const auto loaded = t1.exchange(std::nullopt);
      if (loaded.has_value() == false)
         std::terminate();
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(*loaded - t0).count();
      ready_signal.clear();
      return ns;
   }

}

auto spinlock_latency(const int n) -> void
{
   spinlock.lock();
   just_do_it(n, "spinlock_latency", measure);
}