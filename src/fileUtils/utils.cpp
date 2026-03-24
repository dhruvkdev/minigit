#include "fileUtils/utils.h"
#include "fileUtils/hash.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>

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
  
  std::unordered_map<std::string, std::string> buildSnapshot(const std::string& root){
    // Skipping all the files from .mgitignore 
    std::ifstream f(root + "/.mgitignore");
    std::vector<std::string> ignorables;

    auto trim = [](std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) s.clear();
        else s = s.substr(start, end - start + 1);
    };

    if(f.is_open()){
        std::string s;
        while(getline(f, s)){
            trim(s);
            if(!s.empty()){
                ignorables.push_back(s);
            }
        }
    }

    std::vector<std::string> wilds;
    std::vector<std::string> dirPatterns;
    for(auto& p: ignorables){
      if(p.empty())continue;
        if(p[0] == '*'){
            wilds.push_back(p.substr(1));
        }else if(p.ends_with("/") || p.ends_with("/*")){
          std::string dir = p;
          if(p.ends_with("/*"))dir = p.substr(0, p.size()-1); // removing '*' from the end.
          dirPatterns.push_back(dir);
        }
    }

    std::unordered_map<std::string, std::string> snapshot;

    for(const auto& entry: std::filesystem::recursive_directory_iterator(root)){
        if(entry.is_regular_file()){

            std::string relativePath = std::filesystem::relative(entry.path(), root).string();

            // normalize path
            std::replace(relativePath.begin(), relativePath.end(), '\\', '/');

            if (relativePath == ".mgitignore") continue;
            if (relativePath.starts_with(".mgit")) continue;
            if (relativePath.starts_with(".git")) continue;

            bool found = false;

            for(const auto& p : ignorables){
                if(p == relativePath){
                    found = true;
                    break;
                }
            }

            if(!found){
                for(const auto& p : wilds){
                    if(relativePath.ends_with(p)){
                        found = true;
                        break;
                    }
                }
            }

            if(!found){
              for(const auto& dir: dirPatterns){
                if(relativePath.starts_with(dir)){
                  found = true;
                  break;
                }
              }
            }

            if(found) continue;

            std::ifstream in(entry.path(), std::ios::binary);
            if(!in) continue;

            std::stringstream buffer;
            buffer << in.rdbuf();
            snapshot[relativePath] = buffer.str();
        }
    }

    return snapshot;
  }

  std::string getCurrentDate(){
    // Get the current time point
    auto now = std::chrono::system_clock::now();
    
    // Convert time point to a time_t object
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);

    // Convert to local time tm structure
    std::tm tm = *std::localtime(&now_t);

    std::ostringstream oss;

    oss << std::put_time(&tm, "%d-%m-%Y");
    std::string date = oss.str();
    return date;
  }

  std::string getCurrentTime(){
    // Get the current time point
    auto now = std::chrono::system_clock::now();
    
    // Convert time point to a time_t object
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);

    // Convert to local time tm structure
    std::tm tm = *std::localtime(&now_t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");

    std::string time = oss.str();
    return time;
  }

}
