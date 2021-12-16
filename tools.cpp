#include "tools.h"

auto get_percentile(const std::vector<double>& vec, const double percentile) -> double
{
   std::vector<double> sorted = vec;
   std::ranges::sort(sorted);
   const int perc_index = static_cast<int>(percentile / 100.0 * std::size(sorted));
   return sorted[perc_index];
}


auto goto_horizontal(const int pos) -> std::string
{
   std::string msg = "\x1b[";
   msg += std::to_string(pos);
   msg += "G";
   return msg;
}


auto progress_reporter::get_percent() -> std::optional<std::string>
{
   if (m_last_report.has_value() == false || (std::chrono::high_resolution_clock::now() - *m_last_report) > 50ms) {
      m_last_report = std::chrono::high_resolution_clock::now();
      return std::to_string(100 * m_progress / (m_max - 1)) + "%";;
   }
   return std::nullopt;
}
