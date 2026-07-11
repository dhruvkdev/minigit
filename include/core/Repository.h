#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_set>
#include "core/Commit.h"
#include "core/branch.hxx"

class Repository {
  std::string repoRoot = "";
  std::string repoPath = ".mgit";
  std::string HEAD = "main";
  std::string latestCommit = "";

  Commit loadCommit(const std::string& commitId) const;
  std::string findMergeBase(const std::string& commitA, const std::string& commitB) const;
  bool isAncestorOf(const std::string& ancestor, const std::string& descendant) const;
  void collectAncestors(const std::string& commitId,
                        std::unordered_set<std::string>& out) const;
  void writeWorkingTreeFile(const std::string& relativePath, const std::string& content);
  void removeWorkingTreeFile(const std::string& relativePath);
  std::unordered_map<std::string, std::string> unionFilePaths(
      const std::unordered_map<std::string, std::string>& a,
      const std::unordered_map<std::string, std::string>& b) const;
  void commit(const std::string& message, const std::string& secondParentCommitId);

  public:
    std::string getHEAD() const;
    std::string getBranchCommit(const std::string& branch);
    std::string getCurrentBranch();
    std::string getLatestCommit();

    void init();
    void log();

    void setHEAD(std::string head);
    void updateBranch(std::string branch, std::string commitId);
    void add(std::vector<std::string>& filesToStage);
    void commit(const std::string& message);
    void checkout(std::string branch);
    void createBranch(std::string branch);
    void status();
    void merge(const std::string& targetBranch);
    void diff(const std::string& filePath = "");
    void loadRepo();
    std::string findRepoRoot();

};
