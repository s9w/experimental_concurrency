#include "json_write.h"

#include <fstream>

namespace
{

   [[nodiscard]] auto get_vec_str(const std::vector<curry::result_unit>& vec) -> std::string
   {
      std::string str;
      for (int i_vec = 0; i_vec < std::size(vec); ++i_vec)
      {
         if (i_vec > 0)
            str += ", ";
         str += std::to_string(vec[i_vec]);
      }
      return str;
   }

} // namespace {}

auto curry::json_write(const serialize_type& data, const char* fn) -> void
{
   std::string str;
   str += "{\n";
   int i = 0;
   for (const auto& [key, vec] : data)
   {
      str += "   \"";
      str += key;
      str += "\": [";
      str += get_vec_str(vec);
      str += "]";
      if (i != std::size(data) - 1)
         str += ",";
      str += "\n";
      ++i;
   }
   str += "}\n";

   std::ofstream file(fn);
   file << str;
}
