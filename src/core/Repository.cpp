#include "core/Repository.h"
#include "core/Branch.h"
#include "core/Commit.h"
#include "fileUtils/hash.h"
#include "fileUtils/utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

std::string Repository::getHEAD() const { return HEAD; }
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
  // 1. Create the directories
  fs::create_directories(repoPath + "/commits");
  fs::create_directories(repoPath + "/refs/heads");

  // 2. Create the default "main" branch (currently points to nothing)
  utils::writeToFile(repoPath + "/refs/heads/" + HEAD, "");

  // 3. Set HEAD to point to main
  utils::writeToFile(repoPath + "/HEAD", "ref: refs/heads/" + HEAD);

  std::cout << "Initialized empty MiniGit repository in " << repoPath << "\n";
}

void Repository::commit(const std::string& message){
  std::string commitId = hash::generateHash(message);
  std::string parentCommitId = getLatestCommit();

  std::string date = "Today"; //@TODO: Format Date and Time has to be done using chrono.
  std::string time = "Now";

  std::unordered_map<std::string, std::string> mp; //@TODO: The automation of files in the unordered_map is yet to be done.
  mp["file1.txt"] = "This is the text for file 01.";
  mp["file2.txt"] = "This is the text for file 02.";

  Commit newCommit(commitId, message, parentCommitId, mp, date, time);

  std::string path = repoPath + "/commits/" + commitId + ".bin";
  newCommit.serialize(path); // Commit Object stored in Disk.

  // Commit check = newCommit.deserialize(path);
  //std::cout<< check.getCommitMsg()<<'\n';
  
  //Updating the LatestCommit in the branch from here.
  std::string branch = getCurrentBranch();
  utils::writeToFile(repoPath + "/refs/heads/" + branch, commitId);
}


