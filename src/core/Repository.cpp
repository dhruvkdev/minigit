#include "core/Repository.h"
#include "core/branch.hxx"
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

std::string Repository::getHEAD() const {
    return utils::readFromFile(repoPath + "/HEAD");
}

std::string Repository::getBranchCommit(const std::string& branch) {
    return utils::readFromFile(repoPath + "/refs/heads/" + branch);
}

std::string Repository::getCurrentBranch() {
    std::string head = utils::readFromFile(repoPath + "/HEAD");
    return head.substr(std::string("ref: refs/heads/").size());
}

std::string Repository::getLatestCommit() {
    std::string branch = getCurrentBranch();
    return getBranchCommit(branch);
}

void Repository::init() {
    repoRoot = fs::current_path().string();
    repoPath = repoRoot + "/.mgit";

    fs::create_directories(repoPath + "/commits");
    fs::create_directories(repoPath + "/refs/heads");

    utils::writeToFile(repoPath + "/refs/heads/" + HEAD, "");
    utils::writeToFile(repoPath + "/HEAD", "ref: refs/heads/" + HEAD);

    std::cout << "Initialized empty MiniGit repository in " << repoPath << "\n";
}

void Repository::show(std::string commitId, const std::string& fileName) {
    std::string targetCommit = commitId;
    if (targetCommit == "") {
        targetCommit = getLatestCommit();
    }

    std::string commitPath = repoPath + "/commits/" + targetCommit + ".bin";
    if (!std::filesystem::exists(commitPath)) {
        std::cout << "Commit " << targetCommit << " does not exist!\n";
        return;
    }

    Commit c = Commit::deserialize(commitPath);
    std::unordered_map<std::string, std::string> files = c.getFileBlob();

    if (files.find(fileName) != files.end()) {
        std::cout << files[fileName] << '\n';
    } else {
        std::cout << "Error: File '" << fileName << "' not found in commit " << targetCommit << '\n';
    }
}

void Repository::merge(const std::string& targetBranch) {
    loadRepo();
    std::string currCommitId = getLatestCommit();

    std::string branchPath = repoPath + "/refs/heads/" + targetBranch;
    if (!std::filesystem::exists(branchPath)) {
        std::cout << "Target branch '" << targetBranch << "' does not exist!\n";
        return;
    }

    std::string targetCommitId = getBranchCommit(targetBranch);

    std::string currCommitPath = repoPath + "/commits/" + currCommitId + ".bin";
    std::string targetCommitPath = repoPath + "/commits/" + targetCommitId + ".bin";

    Commit currCommit = Commit::deserialize(currCommitPath);
    Commit targetCommit = Commit::deserialize(targetCommitPath);

    auto currBlob = currCommit.getFileBlob();
    auto targetBlob = targetCommit.getFileBlob();

    for (auto const& [fileName, targetContent] : targetBlob) {
        if (currBlob.find(fileName) != currBlob.end()) {
            if (currBlob[fileName] != targetContent) {
                std::cout << "\nMerge Conflict detected in: " << fileName << "\n";
                std::cout << "1. Accept Incoming Changes\n";
                std::cout << "2. Reject Incoming Changes\n";
                std::cout << "3. Keep Both Changes\n";
                std::cout << "Enter your choice (1/2/3): ";

                std::cout << "\n\n<<<<<<< Current Files Content>>>>>>>\n" 
                          << currBlob[fileName] << '\n';

                std::cout << "\n\n<<<<<<< Incoming Files Content>>>>>>>\n" 
                          << targetContent << '\n';

                int choice;
                std::cin >> choice;

                if (choice == 1) {
                    utils::writeToFile(repoRoot + "/" + fileName, targetContent);
                } else if (choice == 3) {
                    std::string combined = currBlob[fileName] + targetContent;
                    utils::writeToFile(repoRoot + "/" + fileName, combined);
                }
            }
        } else {
            utils::writeToFile(repoRoot + "/" + fileName, targetContent);
        }
    }

    std::vector<std::string> filesToStage = {"."};
    add(filesToStage);
    commit("Merge branch " + targetBranch);

    std::cout << "Successfully merged " << targetBranch 
              << " into " << getCurrentBranch() << "!\n";
}

std::string Repository::findRepoRoot() {
    fs::path current = fs::current_path();

    while (true) {
        if (current.filename() == ".mgit") {
            current = current.parent_path();
        }

        fs::path mgitPath = current / ".mgit";

        if (fs::exists(mgitPath) && fs::is_directory(mgitPath)) {
            return current.string();
        }

        if (current == current.root_path()) break;

        current = current.parent_path();
    }

    throw std::runtime_error("Not a MiniGit repository (or any parent directory)");
}

void Repository::loadRepo() {
    repoRoot = findRepoRoot();
    repoPath = repoRoot + "/.mgit";

    if (!fs::exists(repoPath)) {
        throw std::runtime_error("Not a MiniGit repository");
    }
}

void Repository::add(std::vector<std::string>& filesToStage) {
    std::ofstream indexFile(".mgit/staged", std::ios::trunc);

    if (!indexFile.is_open()) {
        std::cerr << "Fatal: Could not open .mgit/staged for writing.\n";
        return;
    }

    for (const auto& file : filesToStage) {
        indexFile << file << '\n';
        if (file == ".") std::cout << "Staged all the files.\n";
        else std::cout << "Staged: " << file << '\n';
    }

    indexFile.close();
}

void Repository::commit(const std::string& message) {
    loadRepo();

    std::string commitId = hash::generateHash(message);
    std::string parentCommitId = getLatestCommit();

    std::string date = utils::getCurrentDate();
    std::string time = utils::getCurrentTime();

    std::ifstream indexFile(".mgit/staged");
    std::vector<std::string> stagedFiles;
    std::string file;

    while (std::getline(indexFile, file)) {
        if (!file.empty()) stagedFiles.push_back(file);
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
    }

    Commit newCommit(commitId, message, parentCommitId, mp, date, time);

    std::string path = repoPath + "/commits/" + commitId + ".bin";
    newCommit.serialize(path);

    Commit check = newCommit.deserialize(path);
    std::cout << check.getCommitMsg() << '\n';

    for (auto [p, q] : check.getFileBlob()) {
        std::cout << "\n----------------------------------------NEW FILE-------------------------------------\n";
        std::cout << p << ": " << q.substr(0, 100) << "\n";
    }

    std::string branch = getCurrentBranch();
    utils::writeToFile(repoPath + "/refs/heads/" + branch, commitId);
}

void Repository::setHEAD(std::string head) {
    HEAD = head;
    utils::writeToFile(repoPath + "/HEAD", "ref: refs/heads/" + HEAD);
}

void Repository::updateBranch(std::string branch, std::string commitId) {
    std::string path = repoPath + "/refs/heads/" + branch;

    if (std::filesystem::exists(path)) {
        utils::writeToFile(path, commitId);
    } else {
        std::cout << "No such branch exists!\n";
    }
}

void Repository::checkout(std::string branch) {
    std::string path = repoPath + "/refs/heads/" + branch;

    if (std::filesystem::exists(path)) {
        setHEAD(branch);
    } else {
        std::cout << "The branch " << branch << " does not exist!\n";
    }
}

void Repository::createBranch(std::string branch) {
    std::string path = repoPath + "/refs/heads/" + branch;
    utils::writeToFile(path, getLatestCommit());
}

void Repository::status() {
    std::cout << "Current Branch :- " << getCurrentBranch() << '\n';
    std::string s = getLatestCommit();
    if (s == "") {
        std::cout << "No Commits Yet.\n";
    } else {
        std::cout << "Latest Commit :- " << s << '\n';
        std::string commitPath = repoPath + "/commits/" + s + ".bin";
        Commit c = Commit::deserialize(commitPath);
        std::cout << "Date : " << c.getDate() << '\n';
        std::cout << "Time : " << c.getTime() << '\n';
    }
    std::cout << "Staged Files :- \n";
    std::ifstream indexFile(repoPath + "/staged");
    std::string file;
    while (std::getline(indexFile, file)) {
        if (!file.empty()) {
            std::cout << file << '\n';
        }
    }
    indexFile.close();
}

void Repository::log() {
    std::string commitId = getLatestCommit();
    std::string currentBranch = getCurrentBranch();

    while (!commitId.empty()) {
        Commit c = Commit::deserialize(repoPath + "/commits/" + commitId + ".bin");

        std::cout << BOLD << YELLOW << "commit " << c.getCommitId() << RESET;

        if (commitId == getLatestCommit()) {
            std::cout << " " << CYAN << "(HEAD -> " << currentBranch << ")" << RESET;
        }

        std::cout << "\n";

        std::cout << GREEN << "Date: " << RESET << c.getDate() << "\n";
        std::cout << GREEN << "Time: " << RESET << c.getTime() << "\n\n";

        std::cout << "    " << c.getCommitMsg() << "\n\n";
        std::cout << "----------------------------------------\n";

        commitId = c.getParentCommitId();
    }
}