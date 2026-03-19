#include "fileUtils/utils.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace utils{
  void writeString(std::ofstream& out, const std::string& str){
    size_t size = str.size();
    out.write(reinterpret_cast<const char*>(&size), sizeof(size)); //Reinterpret Cast is used to reiterpret the objects. It bypasses the most type-safety checks.
    out.write(str.c_str(), size); //c_str gives a pointer of string type ending with null character '\0'.
  }
  std::string readString(std::ifstream& in){
    size_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));

    std::string str(size, '\0');  // allocate buffer
    in.read(&str[0], size);       // fill it

    return str;
  }
  void writeToFile(const std::string& filepath, const std::string& content){
    std::ofstream file(filepath);
    if(file.is_open()){
      file << content;
      file.close();
    }else{
      std::cerr<<"ERROR: Could not Open the file "<<filepath<<'\n';
    }
  }
  std::string readFromFile(const std::string& filePath){
    std::ifstream file(filePath);
    std::string content, line;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            content += line + "\n"; // Basic reading (can be optimized later)
        }
        file.close();
    }
    // Strip trailing newline for cleaner hashes/pointers
    if (!content.empty() && content.back() == '\n') content.pop_back(); 
    return content;
  }
}
