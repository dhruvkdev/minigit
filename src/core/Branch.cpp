#include "core/Branch.h"
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "fileUtils/utils.h"

Branch::Branch(std::string& branchName, std::string& commitId){
  this->branchName = branchName;
  this->commitId = commitId;
}

std::string Branch::getBranchName() const { return branchName; }
std::string Branch::getCommitId() const { return commitId; }



