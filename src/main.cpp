#include "core/Repository.h"
#include <iostream>

int main(int argc, char* argv[]){
  if (argc < 2) {
      std::cout << "Usage: mgit <command>\n";
      return 1;
  }
  std::string command = argv[1];
  Repository repo;
  if(command == "init"){
    repo.init();
  }else if(command == "commit"){
    if(argc < 3){
      std::cout<<"Commit Message is required\n";
      return 1;
    }
    repo.commit(argv[2]);
  }else if(command == "log"){
    repo.log();
  }else if(command == "branch"){
    if(argc < 3){
      std::cout<<"Branch Name is required.\n";
      return 1;
    }
    repo.createBranch(argv[2]);

  }else if(command == "checkout"){
    if(argc < 3){
      std::cout<<"Branch Name is required.\n";
      return 1;
    }
    repo.checkout(argv[2]);
  }else{
    std::cout<<"Unknown Command.\n";
  }
  return 0;
}
