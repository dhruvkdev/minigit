#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "core/Commit.h"
#include "core/branch.hxx"

class Repository {
    std::string repoRoot = "";
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

    // Show file content from a commit
    void show(std::string commitId, const std::string& fileName);

    void merge(const std::string& targetBranch);
    void setHEAD(std::string head);
    void updateBranch(std::string branch, std::string commitId);

    void add(std::vector<std::string>& filesToStage);
    void commit(const std::string& message);

    void checkout(std::string branch);
    void createBranch(std::string branch);
    
    void status();

    void loadRepo();
    std::string findRepoRoot();
};