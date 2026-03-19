#pragma once
#include <string>

namespace utils{
  void writeString(std::ofstream& out, const std::string& str);
  std::string readString(std::ifstream& in);
  void writeToFile(const std::string& filepath, const std::string& content);
  std::string readFromFile(const std::string& filePath);
}
