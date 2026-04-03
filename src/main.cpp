#include "core/Repository.h"

#include <iostream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: mgit <command>\n";
        return 1;
    }

    std::string command = argv[1];
    Repository repo;

    if (command == "init") {
        repo.init();

    } else if (command == "commit") {
        if (argc < 3) {
            std::cout << "Commit Message is required\n";
            return 1;
        }
        repo.commit(argv[2]);

    } else if (command == "log") {
        repo.log();

    } else if (command == "branch") {
        if (argc < 3) {
            std::cout << "Branch Name is required.\n";
            return 1;
        }
        repo.createBranch(argv[2]);

    } else if (command == "checkout") {
        if (argc < 3) {
            std::cout << "Branch Name is required.\n";
            return 1;
        }
        repo.checkout(argv[2]);

    } else if (command == "add") {
        std::vector<std::string> filesToStage;
        for (int i = 2; i < argc; i++) {
            filesToStage.push_back(argv[i]);
        }
        repo.add(filesToStage);

    } else if (command == "show") {
        if (argc < 4) {
            std::cout << "Commit ID and File Name is required.\n";
            return 1;
        }
        repo.show(argv[2], argv[3]);

    } else if (command == "merge") {
        if (argc < 3) {
            std::cout << "Branch Name is required.\n";
            return 1;
        }
        repo.merge(argv[2]);

    } else if (command == "status") {
        repo.status();

    } else {
        std::cout << "Unknown Command.\n";
    }

    return 0;
}