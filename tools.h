#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <chrono>
using namespace std::chrono_literals;
using dbl_ns = std::chrono::duration<double, std::nano>;
using dbl_ms = std::chrono::duration<double, std::milli>;
using dbl_us = std::chrono::duration<double, std::micro>;

constexpr auto max_threadup_spinup_time = 1.5ms;
constexpr auto max_thread_write_time = 100us;

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
