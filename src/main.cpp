#include "core/Commit.h"
#include "core/Branch.h"
#include "core/Repository.h"
#include <bits/stdc++.h>
using namespace std;

int main(){
  /*
  string commitId = "123456";
  string commitMsg = "This is the message.";
  string parentCommitId = "789456";
  string date = "18 March 2026";
  string time = "19-47-25";
  unordered_map<string, string>mp;
  mp["abc.txt"] = "This is the content of this abc file.";
  Commit commit1(commitId, commitMsg, parentCommitId, mp, date, time);
  string filename = ".mgit/commits/commit1.bin";
  commit1.serialize(filename);
  Commit commit2 = Commit::deserialize(filename);

  std::cout << commit2.getCommitId() << "\n"; 
  string branchName = "main";
  string commitId = "123456789";
  Branch branch1(branchName, commitId);
  string filename = ".mgit/branch/branch1.bin";
  branch1.serialize(filename);
  cout<< branch1.getBranchName()<<" "<< branch1.getCommitId()<<'\n';
  */ 

  //  std::cout<< "Current working directory: "<<std::filesystem::current_path()<<'\n';
  string commitMsg = "This is the sample Commit Message for testing.";
  string commit2 = "This is the second commit message for testing.";
  Repository repo1;
  repo1.init();
  repo1.commit(commitMsg);
  repo1.createBranch("gourav");
  repo1.checkout("gourav");
  repo1.commit(commit2);
  repo1.log();
  return 0;
}
