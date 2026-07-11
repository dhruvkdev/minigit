#include "core/Repository.h"
#include "core/branch.hxx"
#include "core/Commit.h"
#include "fileUtils/diff.h"
#include "fileUtils/hash.h"
#include "fileUtils/utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include "fileUtils/serialization.hxx"

namespace fs = std::filesystem;

#define RESET   "\033[0m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

std::string Repository::getHEAD() const{
  return utils::readFromFile(repoPath + "/HEAD");
} 
std::string Repository::getBranchCommit(const std::string& branch) {
    return utils::readFromFile(repoPath + "/refs/heads/" + branch);
}
std::string Repository::getCurrentBranch() {
    std::string head = utils::readFromFile(repoPath + "/HEAD");

    // head = "ref: refs/heads/main"
    return head.substr(std::string("ref: refs/heads/").size());
}
std::string Repository::getLatestCommit() {
    std::string branch = getCurrentBranch();
    return getBranchCommit(branch);
}
void Repository::init(){
  repoRoot = fs::current_path().string();
  repoPath = repoRoot + "/.mgit";
  //std::cout<<"Repo Root in init is set to: "<<repoRoot<<'\n';
  // 1. Create the directories
  fs::create_directories(repoPath + "/commits");
  fs::create_directories(repoPath + "/refs/heads");

  // 2. Create the default "main" branch (currently points to nothing)
  utils::writeToFile(repoPath + "/refs/heads/" + HEAD, "");

  // 3. Set HEAD to point to main
  utils::writeToFile(repoPath + "/HEAD", "ref: refs/heads/" + HEAD);

  std::cout << "Initialized empty MiniGit repository in " << repoPath << "\n";
}

std::string Repository::findRepoRoot() {
    fs::path current = fs::current_path();

    while (true) {
        // If we're inside .mgit, go one level up
        if (current.filename() == ".mgit") {
            current = current.parent_path();
        }

        fs::path mgitPath = current / ".mgit";

        if (fs::exists(mgitPath) && fs::is_directory(mgitPath)) {
            return current.string();
        }

        if (current == current.root_path()) {
            break;
        }

        current = current.parent_path();
    }

    throw std::runtime_error("Not a MiniGit repository (or any parent directory)");
}

void Repository::loadRepo(){
  repoRoot = findRepoRoot();
  repoPath = repoRoot + "/.mgit";
   if (!fs::exists(repoPath)) {
      throw std::runtime_error("Not a MiniGit repository");
  }
}

Commit Repository::loadCommit(const std::string& commitId) const {
  if (commitId.empty()) {
    return Commit("", "", "", {}, "", "");
  }
  return Serialization::deserialize(repoPath + "/commits/" + commitId + ".bin");
}

void Repository::collectAncestors(const std::string& commitId,
                                  std::unordered_set<std::string>& out) const {
  std::string current = commitId;
  while (!current.empty()) {
    if (!out.insert(current).second) break;
    Commit c = loadCommit(current);
    current = c.getParentCommitId();
    const std::string second = c.getSecondParentCommitId();
    if (!second.empty()) {
      collectAncestors(second, out);
    }
  }
}

bool Repository::isAncestorOf(const std::string& ancestor,
                              const std::string& descendant) const {
  if (ancestor.empty()) return false;
  std::unordered_set<std::string> ancestors;
  collectAncestors(descendant, ancestors);
  return ancestors.count(ancestor) > 0;
}

std::string Repository::findMergeBase(const std::string& commitA,
                                      const std::string& commitB) const {
  if (commitA.empty() || commitB.empty()) return "";
  if (commitA == commitB) return commitA;

  std::unordered_set<std::string> ancestorsA;
  collectAncestors(commitA, ancestorsA);

  std::unordered_set<std::string> visitedB;
  std::vector<std::string> queue = {commitB};
  while (!queue.empty()) {
    std::string current = queue.back();
    queue.pop_back();
    if (current.empty() || !visitedB.insert(current).second) continue;
    if (ancestorsA.count(current) > 0) return current;

    Commit c = loadCommit(current);
    if (!c.getParentCommitId().empty()) queue.push_back(c.getParentCommitId());
    if (!c.getSecondParentCommitId().empty()) {
      queue.push_back(c.getSecondParentCommitId());
    }
  }

  return "";
}

void Repository::writeWorkingTreeFile(const std::string& relativePath,
                                      const std::string& content) {
  fs::path fullPath = fs::path(repoRoot) / relativePath;
  if (fullPath.has_parent_path()) {
    fs::create_directories(fullPath.parent_path());
  }
  std::ofstream out(fullPath, std::ios::binary | std::ios::trunc);
  if (!out.is_open()) {
    throw std::runtime_error("Could not write file: " + fullPath.string());
  }
  out.write(content.c_str(), static_cast<std::streamsize>(content.size()));
}

void Repository::removeWorkingTreeFile(const std::string& relativePath) {
  fs::path fullPath = fs::path(repoRoot) / relativePath;
  if (fs::exists(fullPath)) {
    fs::remove(fullPath);
  }
}

std::unordered_map<std::string, std::string> Repository::unionFilePaths(
    const std::unordered_map<std::string, std::string>& a,
    const std::unordered_map<std::string, std::string>& b) const {
  std::unordered_map<std::string, std::string> result = a;
  for (const auto& [path, content] : b) {
    result[path] = content;
  }
  return result;
}
void Repository::add(std::vector<std::string>& filesToStage)
{
  loadRepo();
  std::ofstream indexFile(repoPath + "/staged", std::ios::app);
  if (!indexFile.is_open()) {
        std::cerr << "Fatal: Could not open " << repoPath << "/staged for writing.\n";
        return;
    }

    for (const auto& file : filesToStage) {
        indexFile << file << '\n';
        if(file == ".") std::cout << "Staged all the files." << '\n';
        else std::cout << "Staged: " << file << '\n';
    }

    indexFile.close();
}
void Repository::commit(const std::string& message){
  commit(message, "");
}

void Repository::commit(const std::string& message, const std::string& secondParentCommitId) {
  loadRepo();

  std::string parentCommitId = getLatestCommit();
  std::string date = utils::getCurrentDate();
  std::string time = utils::getCurrentTime();

  std::ifstream indexFile(repoPath + "/staged");
  std::vector<std::string> stagedFiles;
  std::string file;
  while (std::getline(indexFile, file)) {
    if (!file.empty()) {
      stagedFiles.push_back(file);
    }
  }
  indexFile.close();

  std::unordered_map<std::string, std::string> mp;
  if (stagedFiles.size() == 0) {
    std::cout << "No files staged.\n";
    return;
  } else if (stagedFiles.size() == 1 && stagedFiles[0] == ".") {
    mp = utils::buildSnapshotAll(repoRoot);
  } else {
    mp = utils::buildSnapshot(repoRoot, stagedFiles);
    if (!parentCommitId.empty()) {
      Commit parentCommit = loadCommit(parentCommitId);
      std::unordered_map<std::string, std::string> parent_mp = parentCommit.getFileBlob();
      for (auto [path, content] : parent_mp) {
        if (mp.find(path) != mp.end()) continue;
        mp[path] = content;
      }
    }
  }

  std::ostringstream commitPayload;
  commitPayload << message << '\0' << parentCommitId << '\0' << secondParentCommitId << '\0';
  for (const auto& [path, content] : mp) {
    commitPayload << path << '\0' << content << '\0';
  }
  std::string commitId = hash::generateHash(commitPayload.str());

  Commit newCommit(commitId, message, parentCommitId, mp, date, time,
                   secondParentCommitId);

  std::string path = repoPath + "/commits/" + commitId + ".bin";
  newCommit.serialize(path);

  std::string branch = getCurrentBranch();
  utils::writeToFile(repoPath + "/refs/heads/" + branch, commitId);

  std::ofstream stagingFile(repoPath + "/staged", std::ios::trunc);
  stagingFile.close();
}

void Repository::setHEAD(std::string head){
  HEAD = head;
  utils::writeToFile(repoPath + "/HEAD", "ref: refs/heads/" + HEAD);
}

void Repository::updateBranch(std::string branch, std::string commitId){
  std::string path = repoPath + "/refs/heads/" + branch;
  if(std::filesystem::exists(path)){
    utils::writeToFile(path, commitId);
  }else{
    std::cout<<"No such branch exists!\n";
  }
}
void Repository::status(){
  loadRepo();
  std::cout<<"On branch "<<getCurrentBranch()<<'\n';
  std::string commitId = getLatestCommit();
  if (!commitId.empty()) {
    Commit c = Serialization::deserialize(repoPath + "/commits/" + commitId + ".bin");
    std::cout<<"Date: "<<c.getDate()<<'\n';
    std::cout<<"Time: "<<c.getTime()<<'\n';
  }
  std::cout<<"Changes to be committed:\n";
  std::ifstream indexFile(repoPath + "/staged");
  std::string file;
  while (std::getline(indexFile, file)) 
  {
    if (!file.empty()) 
    {
      std::cout<<file<<'\n';
    }
  }
  indexFile.close();
}

void Repository::merge(const std::string& targetBranch) {
  loadRepo();

  const std::string currentBranch = getCurrentBranch();
  const std::string currCommitId = getLatestCommit();

  const std::string branchPath = repoPath + "/refs/heads/" + targetBranch;
  if (!fs::exists(branchPath)) {
    std::cout << "Target branch '" << targetBranch << "' does not exist!\n";
    return;
  }

  const std::string targetCommitId = getBranchCommit(targetBranch);
  if (targetCommitId.empty()) {
    std::cout << "Target branch '" << targetBranch << "' has no commits.\n";
    return;
  }

  if (currCommitId == targetCommitId) {
    std::cout << "Already up to date.\n";
    return;
  }

  if (!currCommitId.empty() && isAncestorOf(currCommitId, targetCommitId)) {
    utils::writeToFile(repoPath + "/refs/heads/" + currentBranch, targetCommitId);
    checkout(currentBranch);
    std::cout << "Fast-forward\n";
    return;
  }

  if (isAncestorOf(targetCommitId, currCommitId)) {
    std::cout << "Already up to date.\n";
    return;
  }

  Commit currCommit = loadCommit(currCommitId);
  Commit targetCommit = loadCommit(targetCommitId);
  const std::string baseCommitId = findMergeBase(currCommitId, targetCommitId);
  Commit baseCommit = loadCommit(baseCommitId);

  const auto currBlob = currCommit.getFileBlob();
  const auto targetBlob = targetCommit.getFileBlob();
  const auto baseBlob = baseCommit.getFileBlob();

  const auto allPaths = unionFilePaths(unionFilePaths(currBlob, targetBlob), baseBlob);
  bool hasConflicts = false;

  for (const auto& [filePath, _] : allPaths) {
    const bool inBase = baseBlob.count(filePath) > 0;
    const bool inCurr = currBlob.count(filePath) > 0;
    const bool inTarget = targetBlob.count(filePath) > 0;

    const std::string baseContent = inBase ? baseBlob.at(filePath) : "";
    const std::string currContent = inCurr ? currBlob.at(filePath) : "";
    const std::string targetContent = inTarget ? targetBlob.at(filePath) : "";

    if (inCurr && inTarget) {
      if (currContent == targetContent) continue;

      if (inBase && currContent == baseContent) {
        writeWorkingTreeFile(filePath, targetContent);
        continue;
      }
      if (inBase && targetContent == baseContent) {
        writeWorkingTreeFile(filePath, currContent);
        continue;
      }

      diff::MergeResult merged =
          diff::threeWayMerge(baseContent, currContent, targetContent);
      writeWorkingTreeFile(filePath, merged.text);
      if (merged.conflict) {
        hasConflicts = true;
        std::cout << "CONFLICT (content): " << filePath << '\n';
      }
      continue;
    }

    if (inCurr && !inTarget) {
      if (!inBase) continue;
      if (currContent == baseContent) {
        removeWorkingTreeFile(filePath);
      } else {
        hasConflicts = true;
        std::cout << "CONFLICT (modify/delete): " << filePath << '\n';
        writeWorkingTreeFile(filePath, currContent);
      }
      continue;
    }

    if (!inCurr && inTarget) {
      if (!inBase) {
        writeWorkingTreeFile(filePath, targetContent);
        continue;
      }
      if (targetContent == baseContent) {
        removeWorkingTreeFile(filePath);
      } else {
        hasConflicts = true;
        std::cout << "CONFLICT (modify/delete): " << filePath << '\n';
        writeWorkingTreeFile(filePath, targetContent);
      }
      continue;
    }

    if (inBase && !inCurr && !inTarget) {
      removeWorkingTreeFile(filePath);
    }
  }

  if (hasConflicts) {
    std::cout << "Automatic merge failed; fix conflicts and then commit the result.\n";
    return;
  }

  std::vector<std::string> filesToStage = {"."};
  add(filesToStage);
  commit("Merge branch '" + targetBranch + "'", targetCommitId);
  std::cout << "Merge made by the 'recursive' strategy.\n";
}

void Repository::diff(const std::string& filePath) {
  loadRepo();

  const std::string commitId = getLatestCommit();
  if (commitId.empty()) {
    std::cout << "No commits yet.\n";
    return;
  }

  Commit headCommit = loadCommit(commitId);
  const auto blob = headCommit.getFileBlob();

  if (filePath.empty()) {
    for (const auto& [path, content] : blob) {
      const std::string working = utils::readFromFile(repoRoot + "/" + path);
      if (working != content) {
        std::cout << diff::formatUnified("a/" + path, "b/" + path, content, working);
      }
    }
    return;
  }

  if (blob.find(filePath) == blob.end()) {
    std::cout << "File not tracked: " << filePath << '\n';
    return;
  }

  const std::string working = utils::readFromFile(repoRoot + "/" + filePath);
  std::cout << diff::formatUnified("a/" + filePath, "b/" + filePath, blob.at(filePath),
                                   working);
}

void Repository::checkout(std::string branch){
  loadRepo();
  std::string path = repoPath + "/refs/heads/" + branch;
  if(!std::filesystem::exists(path)){
    std::cout<<"The branch "<<branch<<" does not exist!\n";
    return;
  }

  //Clearning old branch files
  std::string currentCommitId = getLatestCommit();
  if (!currentCommitId.empty()) 
  {
    std::string currentCommitPath = repoPath + "/commits/" + currentCommitId + ".bin";
    if (std::filesystem::exists(currentCommitPath)) {
      Commit currentCommit = Serialization::deserialize(currentCommitPath);
      for (auto const& [file, content] : currentCommit.getFileBlob()) {
        std::filesystem::path fullPath = std::filesystem::path(findRepoRoot()) / file;
        if (std::filesystem::exists(fullPath)) {
            std::filesystem::remove(fullPath); 
        }
      }
    }
  }

  setHEAD(branch);
  std::string branchCommitId = getBranchCommit(branch);
  if(branchCommitId.empty()) return;
  std::string branchCommitFilePath = repoPath + "/commits/" + branchCommitId + ".bin";
  Commit branchCommit = Serialization::deserialize(branchCommitFilePath);
  std::unordered_map<std::string, std::string> branch_mp = branchCommit.getFileBlob();
  if(branch_mp.empty())
  {
    std::cout<<"EMPTY\n";
    return;
  }

  for(auto& [file, content] : branch_mp)
  {
    repoRoot = findRepoRoot();
    std::filesystem::path fullPath = std::filesystem::path(repoRoot) / file;
    if (fullPath.has_parent_path())
    {
      std::filesystem::create_directories(fullPath.parent_path());
    }

    std::ofstream outFile(fullPath, std::ios::binary | std::ios::trunc);
    std::cout<<file<<" "<<content<<'\n';
    if (outFile.is_open()) 
    {
      outFile.write(content.c_str(), content.size());
      outFile.close();
    }
    else 
    {
      std::cerr << "Fatal: Could not open file for restoring: " << fullPath << "\n";
    }
  }
}

void Repository::createBranch(std::string branch){
  loadRepo();
  std::string path = repoPath + "/refs/heads/" + branch;
  utils::writeToFile(path, getLatestCommit());
}

void Repository::log() {
    loadRepo();
    std::string commitId = getLatestCommit();
    std::string currentBranch = getCurrentBranch();

    while (!commitId.empty()) {
        Commit c = Serialization::deserialize(repoPath + "/commits/" + commitId + ".bin");

        /* 
        for(auto& [p, q]: c.getFileBlob()){
          std::cout<<p<<'\n';
          std::cout<<q.substr(0,100);
        }
        */ 

        // Commit header
        std::cout << BOLD << YELLOW << "commit " << c.getCommitId() << RESET;

        // Show HEAD pointer
        if (commitId == getLatestCommit()) {
            std::cout << " " << CYAN << "(HEAD -> " << currentBranch << ")" << RESET;
        }

        std::cout << "\n";

        // Date
        std::cout << GREEN << "Date: " << RESET
                  << c.getDate() << "\n";

        //Time 
        std::cout << GREEN <<"Time: " << RESET
                  << c.getTime() << " " << "\n\n";

        // Message (indented like git)
        std::cout << "    " << c.getCommitMsg() << "\n\n";

        // Separator
        std::cout << "----------------------------------------\n";

        commitId = c.getParentCommitId();
    }
}
