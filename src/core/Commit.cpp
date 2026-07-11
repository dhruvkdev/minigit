#include <string>
#include "core/Commit.h"
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "fileUtils/utils.h"
Commit::Commit(const std::string& commitId,
               const std::string& commitMsg,
               const std::string& parentCommitId,
               const std::unordered_map<std::string, std::string>& file,
               const std::string& date,
               const std::string& time,
               const std::string& secondParentCommitId)
{
  this->commitId = commitId;
  this->commitMsg = commitMsg;
  this->parentCommitId = parentCommitId;
  this->secondParentCommitId = secondParentCommitId;
  this->fileBlob = file;
  this->date = date;
  this->time = time;
}

std::string Commit::getCommitId() const{ return commitId; }
std::string Commit::getCommitMsg() const { return commitMsg; }
std::string Commit::getParentCommitId() const { return parentCommitId; }
std::string Commit::getSecondParentCommitId() const { return secondParentCommitId; }
std::string Commit::getDate() const { return date; }
std::string Commit::getTime() const { return time; }
std::unordered_map<std::string, std::string> Commit::getFileBlob() const { return fileBlob; }



void Commit::serialize(const std::string& filename){
  std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
  std::ofstream file(filename, std::ios::binary);
  if(file.is_open()){
    const uint32_t magic = 0x4749544D;  // 'MGIT'
    const uint16_t version = 2;
    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));

    utils::writeString(file, commitId);
    utils::writeString(file, commitMsg);
    utils::writeString(file, parentCommitId);
    utils::writeString(file, secondParentCommitId);
    utils::writeString(file, date);
    utils::writeString(file, time);
    size_t mapSize = fileBlob.size();
    file.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
    for(auto& [key, value]: fileBlob){
      utils::writeString(file, key);
      utils::writeString(file, value);
    }
    std::cout<<"Object serialized successfully.\n";
  }else{
    std::cerr<<"ERROR: Failed to open the file for writing.\n";
  }
}
