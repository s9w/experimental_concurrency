#include "spinlock_latency.h"

#include <semaphore>
#include <thread>

#include "tools.h"


namespace {
   class atomic_flag_spinlock {
      std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
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
         //lock_.notify_one();
      }
   };

   atomic_flag_spinlock spinlock;
   //tas_lock spinlock;
   std::atomic<std::optional<std::chrono::high_resolution_clock::time_point>> t1;

   auto locker() -> void {
      spinlock.lock();
      const auto time = std::chrono::high_resolution_clock::now();
      t1.store(time);
   }

   auto measure() -> double {
      std::jthread j(locker);
      std::this_thread::sleep_for(max_threadup_spinup_time);

      // Signal thread and time
      const auto t0 = std::chrono::high_resolution_clock::now();
      spinlock.unlock();
      std::this_thread::sleep_for(max_thread_write_time);
      const auto opt = t1.load();
      if (opt.has_value() == false)
         std::terminate();
      const double ns = std::chrono::duration_cast<dbl_ns>(*opt - t0).count();

      t1.store(std::nullopt);
      return ns;
   }

}

auto measure_spinlock_latency(const int n) -> void
{
   spinlock.lock();
   std::vector<double> runtimes;
   runtimes.reserve(n);
   for (int i = 0; i < n; ++i) {
      if (i % 100 == 0)
         std::cout << oof::hposition(0) << 100 * i / n << "%";
      runtimes.emplace_back(measure());
   }
   std::cout << "\n";
   report(runtimes, "spinlock_latency");
}