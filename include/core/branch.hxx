#pragma once

#include <string>

class Branch {
    std::string branchName;
    std::string commitId; // The most recent commitId in this branch

public:
    Branch(std::string& branchName, std::string& commitId);

    std::string getBranchName() const;
    std::string getCommitId() const;
};