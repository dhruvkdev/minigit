#pragma once
#include <string>

class Branch{
  std::string branchName;
  std::string commitId; //The recentmost commitId in this branch.
  public:
    Branch(std::string& branchName, std::string& commitId);
    std::string getBranchName() const;
    std::string getCommitId() const;

};


