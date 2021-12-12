#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <chrono>
using namespace std::chrono_literals;
using dbl_ns = std::chrono::duration<double, std::nano>;
using dbl_ms = std::chrono::duration<double, std::milli>;

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>


inline auto max_threadup_spinup_time = 5.0ms;
constexpr auto max_thread_write_time = 100us;

struct paired_time{
   int number0{};
   int number1{};
   double ns{};
};

inline auto report(
   const std::vector<paired_time>& vec,
   const char* description
) -> void
{
   std::cout << "written " << description << "\n";
   std::string filename = "python/";
   filename += description;
   filename += ".txt";

   std::ofstream filestream(filename);
   for (const paired_time& value : vec)
      filestream << std::to_string(value.ns) << " " << std::to_string(value.number0) << " " << std::to_string(value.number1) << "\n";
}

inline auto report(
   const std::vector<double>& vec,
   const char* description
) -> void
{
   std::cout << "written " << description << "\n";
   std::string filename = "python/";
   filename += description;
   filename += ".txt";
   
   std::ofstream filestream(filename);
   for (const double value : vec)
      filestream << std::to_string(value) << "\n";
}

inline auto report(
   const std::vector<std::pair<double, double>>& vec,
   const char* description
) -> void
{
   std::cout << "written " << description << "\n";
   std::string filename = "python/";
   filename += description;
   filename += ".txt";

   std::ofstream filestream(filename);
   for (const auto& [row0, row1]: vec)
      filestream << std::to_string(row0) << " " << row1 << "\n";
}
