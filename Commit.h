#pragma once
#include <string>
#include <unordered_map>
class Commit{
  std::string commitId;
  std::string commitMsg;
  std::string parentCommitId;
  std::unordered_map<std::string, std::string> fileBlob;
  std::string date;
  std::string time;

  public:
    Commit(const std::string& commitId,
       const std::string& commitMsg,
       const std::string& parentCommitId,
       const std::unordered_map<std::string, std::string>& file,
       const std::string& date,
       const std::string& time);
    std::string getCommitId() const;
    std::string getCommitMsg() const;
    std::string getParentCommitId() const;
    std::unordered_map<std::string, std::string> getFileBlob() const;
    std::string getDate() const;
    std::string getTime() const;

    //We serialize and Deserialize to store the object in disk, in the .mgit folder later.
    void serialize(const std::string& filename);
    static Commit deserialize(const std::string& filename);
};
