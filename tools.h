#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <chrono>
using namespace std::chrono_literals;
using result_unit = typename std::chrono::nanoseconds::rep;


inline auto max_threadup_spinup_time = 5.0ms;
constexpr auto max_latency = 50us;

struct console_cursor_disabler{
   console_cursor_disabler() { std::cout << "\x1b[?25l";}
   ~console_cursor_disabler(){ std::cout << "\x1b[?25h";}
};


template<typename fun_type>
auto just_do_it(
   const int n,
   const char* description,
   const fun_type& get_measurement
) -> void
{
   std::vector<result_unit> runtimes;
   runtimes.reserve(n);
   {
      console_cursor_disabler no_cursor;
      for (int i = 0; i < n; ++i) {
         if (i % 100 == 0) {
            const int percentage = 100 * i / n;
            std::cout << "\x1b[0G" << percentage << "%    ";
         }
         runtimes.emplace_back(get_measurement());
      }
   }
   std::cout << "\n";

   // Result writing
   std::string filename = "python/";
   filename += description;
   filename += ".txt";

   std::ofstream file_writer(filename);
   for (const result_unit value : runtimes)
      file_writer << std::to_string(value) << "\n";
   std::cout << description << " done\n";
}


//inline auto write_results(
//   const std::vector<paired_time>& vec,
//   const char* description
//) -> void
//{
//   std::cout << "written " << description << "\n";
//   std::string filename = "python/";
//   filename += description;
//   filename += ".txt";
//
//   std::ofstream filestream(filename);
//   for (const paired_time& value : vec)
//      filestream << std::to_string(value.ns) << " " << std::to_string(value.number0) << " " << std::to_string(value.number1) << "\n";
//}
//
//inline auto write_results(
//   const std::vector<double>& vec,
//   const char* description
//) -> void
//{
//   std::cout << "written " << description << "\n";
//   std::string filename = "python/";
//   filename += description;
//   filename += ".txt";
//   
//   std::ofstream filestream(filename);
//   for (const double value : vec)
//      filestream << std::to_string(value) << "\n";
//}
//
//inline auto write_results(
//   const std::vector<std::pair<double, double>>& vec,
//   const char* description
//) -> void
//{
//   std::cout << "written " << description << "\n";
//   std::string filename = "python/";
//   filename += description;
//   filename += ".txt";
//
//   std::ofstream filestream(filename);
//   for (const auto& [row0, row1]: vec)
//      filestream << std::to_string(row0) << " " << row1 << "\n";
//}
