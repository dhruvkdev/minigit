#pragma once
#include <string>
#include <filesystem>
#include "core/Commit.h"
#include "core/Branch.h"

class Repository {
  std::string repoPath = ".mgit";
  std::string HEAD = "main";
  std::string latestCommit = "";
  public:
    std::string getHEAD() const;
    std::string getBranchCommit(const std::string& branch);
    std::string getCurrentBranch();
    std::string getLatestCommit();

    void init();
    void log();

    void setHEAD(std::string head);
    void updateBranch(std::string branch, std::string commitId);
    void commit(const std::string& message);
    void checkout(std::string branch);
    void createBranch(std::string branch);
};
