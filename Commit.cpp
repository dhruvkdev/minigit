#include <string>
#include "Commit.h"
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <filesystem>
Commit::Commit(const std::string& commitId,
               const std::string& commitMsg,
               const std::string& parentCommitId,
               const std::unordered_map<std::string, std::string>& file,
               const std::string& date,
               const std::string& time)
{
  this->commitId = commitId;
  this->commitMsg = commitMsg;
  this->parentCommitId = parentCommitId;
  this->fileBlob = file;
  this->date = date;
  this->time = time;
}

std::string Commit::getCommitId() const{ return commitId; }
std::string Commit::getCommitMsg() const { return commitMsg; }
std::string Commit::getParentCommitId() const { return parentCommitId; }
std::string Commit::getDate() const { return date; }
std::string Commit::getTime() const { return time; }
std::unordered_map<std::string, std::string> Commit::getFileBlob() const { return fileBlob; }


void writeString(std::ofstream& out, const std::string& str){
  size_t size = str.size();
  out.write(reinterpret_cast<const char*>(&size), sizeof(size)); //Reinterpret Cast is used to reiterpret the objects. It bypasses the most type-safety checks.
  out.write(str.c_str(), size); //c_str gives a pointer of string type ending with null character '\0'.
}

void Commit::serialize(const std::string& filename){
  std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
  std::ofstream file(filename, std::ios::binary);
  if(file.is_open()){
    writeString(file, commitId);
    writeString(file, commitMsg);
    writeString(file, parentCommitId);
    writeString(file, date);
    writeString(file, time);
    size_t mapSize = fileBlob.size();
    file.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
    for(auto& [key, value]: fileBlob){
      writeString(file, key);
      writeString(file, value);
    }
    std::cout<<"Object serialized successfully.\n";
  }else{
    std::cerr<<"ERROR: Failed to open the file for writing.\n";
  }
}

std::string readString(std::ifstream& in){
    size_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));

    std::string str(size, '\0');  // allocate buffer
    in.read(&str[0], size);       // fill it

    return str;
}

Commit Commit::deserialize(const std::string& filename){
    std::ifstream in(filename, std::ios::binary);

    if(!in.is_open()){
        throw std::runtime_error("Failed to open file for reading"); //@TODO: Implement the Try-Catch block for this later.
    }

    std::string commitId = readString(in);
    std::string commitMsg = readString(in);
    std::string parentCommitId = readString(in);
    std::string date = readString(in);
    std::string time = readString(in);

    size_t mapSize;
    in.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    std::unordered_map<std::string, std::string> fileBlob;

    for(size_t i = 0; i < mapSize; i++){
        std::string key = readString(in);
        std::string value = readString(in);
        fileBlob[key] = value;
    }

    in.close();

    return Commit(commitId, commitMsg, parentCommitId, fileBlob, date, time);
}
