#pragma once
#include <string>
#include <unordered_map>

namespace utils{
  void writeString(std::ofstream& out, const std::string& str);
  std::string readString(std::ifstream& in);
  void writeToFile(const std::string& filepath, const std::string& content);
  std::string readFromFile(const std::string& filePath);
  std::unordered_map<std::string, std::string>buildSnapshot(const std::string& root);

  std::string getCurrentDate();
  std::string getCurrentTime();
}
