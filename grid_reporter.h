#pragma once

#include <string>

namespace excon
{

   struct grid_reporter {
      int m_core_count = 0;

      explicit grid_reporter(const int core_count);
      ~grid_reporter();

      auto mark_cores_as_inprogress(int core_a, int core_b) const -> void;
      auto mark_cores_as_done(int core_a, int core_b) const -> void;
      auto relative_write(int right, int down, const std::string& str) const -> void;

      grid_reporter(grid_reporter&&) = delete;
      grid_reporter(const grid_reporter&) = delete;
      auto operator=(grid_reporter&&) -> grid_reporter & = delete;
      auto operator=(const grid_reporter&) -> grid_reporter & = delete;
   };

}
