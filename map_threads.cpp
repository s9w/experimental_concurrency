#include "map_threads.h"

#include <semaphore>
#include <random>

#include "oof.h"

namespace
{
   using namespace curry;

   std::atomic_flag start_signal;
   easy_atomic<std::chrono::high_resolution_clock::time_point> t1_atomic;

   std::atomic_int platform{0};

   constexpr int pings = 30;

   auto thread_fun0(std::latch& ready_signal, const int cpu_num) -> void {
      set_thread_affinity({ cpu_num });
      ready_signal.count_down();
      start_signal.wait(false);

      for(int i=0; i<pings; ++i)
      {
         platform.wait(1);
         platform.store(1);
         platform.notify_one();
      }
      platform.wait(1);
      const auto time = std::chrono::high_resolution_clock::now();
      t1_atomic.store_and_notify_one(time);
   }

   auto thread_fun1(std::latch& ready_signal, const int cpu_num) -> void {
      set_thread_affinity({ cpu_num });
      ready_signal.count_down();
      start_signal.wait(false);

      for (int i = 0; i < pings; ++i)
      {
         platform.wait(0);
         platform.store(0);
         platform.notify_one();
      }
      // const auto time = std::chrono::high_resolution_clock::now();
      // t1_atomic.store_and_notify_one(time);
   }


   auto measure(const int cpu_a, const int cpu_b) -> result_unit {
      start_signal.clear();
      std::latch latch(2);
      std::jthread j0(thread_fun0, std::ref(latch), cpu_a);
      std::jthread j1(thread_fun1, std::ref(latch), cpu_b);
      latch.wait();

      const auto t0 = std::chrono::high_resolution_clock::now();
      start_signal.test_and_set();
      start_signal.notify_all();
      const auto t1 = t1_atomic.wait_for_non_nullopt_and_exchange();
      platform.store(0);
      return (t1 - t0).count()/(2*pings);
   }



   [[nodiscard]] auto get_latency(const int cpu_a, const int cpu_b) -> result_unit
   {
      std::vector<result_unit> times;
      times.reserve(5000);
      for (int i = 0; i < 5000; ++i) {
         const auto timing = measure(cpu_a, cpu_b);

         // Toss the first measurement
         if(i==0)
            continue;

         times.emplace_back(timing);
      }
      return get_50th_percentile(times);
   }


   [[nodiscard]] auto get_random_indices(
      const int core_count
   ) -> std::vector<size_t>
   {
      const int seed = 0;
      std::mt19937_64 rng(seed);

      std::vector<size_t> indices;
      indices.reserve(core_count * core_count);
      for (int i = 0; i < core_count * core_count; ++i)
         indices.push_back(i);

      std::ranges::shuffle(indices, rng);
      return indices;
   }


   [[nodiscard]] auto pick_another_thread(
      const int current_thread,
      const int cpu_a,
      const int cpu_b,
      const int core_count
   ) -> std::optional<int>
   {
      if (current_thread != cpu_a && current_thread != cpu_b)
         return std::nullopt;
      for(int i=0; i<core_count; ++i)
      {
         if (i != current_thread && i != cpu_a && i != cpu_b)
            return i;
      }
      std::terminate();
   }

   struct relative_mover {
      int m_right{};
      int m_down{};
      explicit relative_mover(const int right, const int down)
         : m_right(right)
         , m_down(down)
      {
         std::string msg;
         if (right > 0)
            msg += oof::move_right(right);
         if (down > 0)
            msg += oof::move_down(down);
         std::cout << msg;
      }

      ~relative_mover(){
         std::string msg;
         msg += oof::move_left(m_right+1);
         if (m_down > 0)
            msg += oof::move_up(m_down);
         std::cout << msg;
      }
   };

   struct map_progress_reporter{
      int m_core_count = 0;

      explicit map_progress_reporter(const int core_count)
         : m_core_count(core_count)
      {
         std::string str = oof::cursor_visibility(false);
         for (int i = 0; i < m_core_count; ++i) {
            for (int j = 0; j < m_core_count; ++j)
               str += "- ";
            str += "\n";
         }

         str += oof::move_up(m_core_count);
         str += oof::hposition(0);
         std::cout << str;
      }

      ~map_progress_reporter(){
         std::cout << oof::move_down(m_core_count);
      }

      auto run(
         const int cpu_a,
         const int cpu_b
      ) const -> void
      {
         {
            relative_mover m{ 2*cpu_a, cpu_b };
            std::cout << "X";
         }
         {
            relative_mover m{ 2*cpu_b, cpu_a };
            std::cout << "X";
         }
      }
   };

}


auto curry::map_threads(serialize_type& data) -> void
{
   std::cout << "mapping:\n";
   int current_thread = get_logical_processor_number();
   const int core_count = static_cast<int>(std::thread::hardware_concurrency());

   const map_progress_reporter reporter(core_count);

   // Measured CPU cores are measured in randomized order to prevent increased heat to introduce
   // a systematic error
   std::vector<result_unit> times(core_count * core_count);

   for(const size_t index : get_random_indices(core_count))
   {
      const int cpu_a = static_cast<int>(index % core_count);
      const int cpu_b = static_cast<int>(index / core_count);

      if(cpu_b >= cpu_a)
         continue;
      constexpr auto x = sizeof(std::vector<int>) + sizeof(int);
      // Make sure that we're not running on the two targeted threads. This will often pick the
      // fastest CORE (id 12 on my system). Move this thread away.
      const std::optional<int> new_thread = pick_another_thread(current_thread, cpu_a, cpu_b, core_count);
      if(new_thread.has_value())
      {
         set_thread_affinity({new_thread.value()});
         current_thread = new_thread.value();
      }

      times[index] = get_latency(cpu_a, cpu_b);
      reporter.run(cpu_a, cpu_b);
   }
   
   data["thread_map"] = times;
}
