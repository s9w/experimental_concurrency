#include "grid_reporter.h"

#include <iostream>

#include "oof.h"


excon::grid_reporter::grid_reporter(const int core_count): m_core_count(core_count)
{
   std::string str = oof::cursor_visibility(false);
   str += oof::fg_color(oof::color{ 255, 100, 100 });
   for (int i = 0; i < m_core_count; ++i) {
      for (int j = 0; j < m_core_count; ++j) {
         str += "- ";
      }
      str += "\n";
   }

   str += oof::move_up(m_core_count);
   str += oof::reset_formatting();
   std::cout << str;
}


excon::grid_reporter::~grid_reporter()
{
   std::cout << oof::move_down(m_core_count);
}


auto excon::grid_reporter::relative_write(
   const int right,
   const int down,
   const std::string& str
) const -> void
{
   std::string msg;
   if (right > 0)
      msg += oof::move_right(right);
   if (down > 0)
      msg += oof::move_down(down);

   msg += str;

   msg += oof::move_left(right + static_cast<int>(str.size()));
   if (down > 0)
      msg += oof::move_up(down);
   std::cout << msg;
}


auto excon::grid_reporter::mark_cores_as_inprogress(
   const int core_a,
   const int core_b
) const -> void
{
   std::cout << oof::fg_color(oof::color{ 255, 255, 100 });
   relative_write(2 * core_a, core_b, "X");
   relative_write(2 * core_b, core_a, "X");
   std::cout << oof::reset_formatting();
}


auto excon::grid_reporter::mark_cores_as_done(
   const int core_a,
   const int core_b
) const -> void
{
   std::cout << oof::fg_color(oof::color{ 100, 255, 100 });
   relative_write(2 * core_a, core_b, "X");
   relative_write(2 * core_b, core_a, "X");
   std::cout << oof::reset_formatting();
}
