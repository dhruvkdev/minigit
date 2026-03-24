#include "core/Repository.h"
#include "core/Branch.h"
#include "core/Commit.h"
#include "fileUtils/hash.h"
#include "fileUtils/utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <stdexcept>

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

void Repository::commit(const std::string& message){
  loadRepo();

  std::string commitId = hash::generateHash(message);
  std::string parentCommitId = getLatestCommit();

  std::string date = utils::getCurrentDate();
  std::string time = utils::getCurrentTime();

  auto mp = utils::buildSnapshot(repoRoot);

  Commit newCommit(commitId, message, parentCommitId, mp, date, time);

  std::string path = repoPath + "/commits/" + commitId + ".bin";
  newCommit.serialize(path); // Commit Object stored in Disk.

  Commit check = newCommit.deserialize(path);
  //std::cout<< check.getCommitMsg()<<'\n';
  /*
  for(auto [p, q]: check.getFileBlob()){
    std::cout<<"\n\n----------------------------------------NEW FILE-------------------------------------\n\n";
    std::cout << p << ": " << q.substr(0, 100) << "\n";
  }
  */ 

  //Updating the LatestCommit in the branch from here.
  std::string branch = getCurrentBranch();
  utils::writeToFile(repoPath + "/refs/heads/" + branch, commitId);
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

void Repository::checkout(std::string branch){
  std::string path = repoPath + "/refs/heads/" + branch;
  if(std::filesystem::exists(path)){
    setHEAD(branch);
  }else{
    std::cout<<"The branch "<<branch<<" does not exist!\n";
  }
}

void Repository::createBranch(std::string branch){
  std::string path = repoPath + "/refs/heads/" + branch;
  utils::writeToFile(path, getLatestCommit());
}

void Repository::log() {
    std::string commitId = getLatestCommit();
    std::string currentBranch = getCurrentBranch();

    while (!commitId.empty()) {
        Commit c = Commit::deserialize(repoPath + "/commits/" + commitId + ".bin");

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
