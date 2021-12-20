#include "tools.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "oof.h"


namespace
{

   [[nodiscard]] auto get_colored_perc_str(const double progress) -> std::string
   {
      std::string msg;
      const auto red_component = static_cast<int>(255 - progress * 255);
      msg += oof::fg_color(oof::color{ red_component, 255, 0 });
      const int int_precent = static_cast<int>(std::round(100.0 * progress));
      msg += std::to_string(int_precent) + "%";
      msg += oof::reset_formatting();
      return msg;
   }

} // namespace {}


auto curry::get_horizontal_pos_str(const int pos) -> std::string
{
   std::string msg = "\x1b[";
   msg += std::to_string(pos);
   msg += "G";
   return msg;
}


auto curry::set_thread_affinity(const std::vector<int>& cpus) -> void
{
   if (cpus.empty())
      std::terminate();
   const unsigned int core_count = std::thread::hardware_concurrency();
   DWORD_PTR mask = 0;
   for (const int cpu_id : cpus)
   {
      if (cpu_id >= static_cast<int>(core_count))
         std::terminate();
      mask |= 1ui64 << cpu_id;
   }
   HANDLE const thread_handle = GetCurrentThread();
   const DWORD_PTR previous_mask = SetThreadAffinityMask(thread_handle, mask);
   if (previous_mask == 0)
   {
      std::terminate();
   }
}


auto curry::get_logical_processor_number() -> int
{
   const int logical_processor_num = static_cast<int>(GetCurrentProcessorNumber());
   return logical_processor_num;
}


auto curry::get_50th_percentile(
   const std::vector<result_unit>& vec
) -> result_unit
{
   std::vector<result_unit> sorted = vec;
   std::ranges::sort(sorted);
   const size_t index = std::size(vec) / 2;
   return sorted[index];
}


curry::progress_reporter::progress_reporter(
   const std::string& description,
   const int max
)
   : m_max(max)
   , m_progress_pos(static_cast<int>(description.size()) + 3)
{
   std::cout << description << ":";
}


auto curry::progress_reporter::report() -> void
{
   const std::optional<std::string> percent = get_percent();
   if (percent.has_value())
      std::cout << get_horizontal_pos_str(m_progress_pos) << *percent;
   ++m_progress;
}


auto curry::progress_reporter::get_percent() -> std::optional<std::string>
{
   using namespace std::chrono_literals;
   if (m_last_report_time.has_value() == false || (std::chrono::high_resolution_clock::now() - *m_last_report_time) > 100ms)
   {
      m_last_report_time = std::chrono::high_resolution_clock::now();

      const double progress = static_cast<double>(m_progress) / (m_max - 1);
      return get_colored_perc_str(progress);
   }
   return std::nullopt;
}


curry::progress_reporter::~progress_reporter()
{
   std::cout << get_horizontal_pos_str(m_progress_pos) << get_colored_perc_str(1.0) << "\n";
}


template <typename T>
auto curry::easy_atomic<T>::wait_for_non_nullopt() const noexcept -> void
{
   m_atomic.wait(zero_value);
}


template <typename T>
auto curry::easy_atomic<T>::wait_for_non_nullopt_and_exchange(
   std::memory_order order
) noexcept -> T
{
   m_atomic.wait(zero_value, order);
   return *m_atomic.exchange(zero_value, order);
}


template <typename T>
auto curry::easy_atomic<T>::store_and_notify_one(
   T new_value,
   std::memory_order order
) noexcept -> void
{
   m_atomic.store(new_value, order);
   m_atomic.notify_one();
}


template <typename T>
auto curry::easy_atomic<T>::store_and_notify_all(
   T new_value,
   std::memory_order order
) noexcept -> void
{
   m_atomic.store(new_value, order);
   m_atomic.notify_all();
}


template curry::easy_atomic<std::chrono::high_resolution_clock::time_point>;
