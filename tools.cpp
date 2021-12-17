#include "tools.h"


auto get_horizontal_pos_str(const int pos) -> std::string
{
   std::string msg = "\x1b[";
   msg += std::to_string(pos);
   msg += "G";
   return msg;
}


progress_reporter::progress_reporter(const std::string& description, const int max)
   : m_max(max)
   , m_progress_pos(static_cast<int>(description.size()) + 3)
{
   std::cout << description << ":";
}


auto progress_reporter::report() -> void
{
   const std::optional<std::string> percent = get_percent();
   if (percent.has_value())
      std::cout << get_horizontal_pos_str(m_progress_pos) << *percent;
   ++m_progress;
}


auto progress_reporter::get_percent() -> std::optional<std::string>
{
   if (m_last_report.has_value() == false || (std::chrono::high_resolution_clock::now() - *m_last_report) > 100ms)
   {
      m_last_report = std::chrono::high_resolution_clock::now();
      return std::to_string(100 * m_progress / (m_max - 1)) + "%";;
   }
   return std::nullopt;
}


progress_reporter::~progress_reporter()
{
   std::cout << get_horizontal_pos_str(m_progress_pos) << "100%\n";
}


template <typename T>
auto easy_atomic<T>::wait_for_non_nullopt() const noexcept -> void
{
   m_atomic.wait(zero_value);
}

template <typename T>
auto easy_atomic<T>::wait_for_non_nullopt_and_exchange(std::memory_order order) noexcept -> T
{
   m_atomic.wait(zero_value, order);
   return *m_atomic.exchange(zero_value, order);
}


template <typename T>
auto easy_atomic<T>::store_and_notify_one(T new_value, std::memory_order order) noexcept -> void
{
   m_atomic.store(new_value, order);
   m_atomic.notify_one();
}


template <typename T>
auto easy_atomic<T>::store_and_notify_all(T new_value, std::memory_order order) noexcept -> void
{
   m_atomic.store(new_value, order);
   m_atomic.notify_all();
}

template easy_atomic<std::chrono::high_resolution_clock::time_point>;
