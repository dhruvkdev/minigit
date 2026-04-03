#include "fileUtils/serialization.hxx"
#include <iostream>
#include <filesystem>
#include <string>
#include "core/Commit.h"
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "fileUtils/utils.h"


Commit Serialization::deserialize(const std::string& filename){
    std::ifstream in(filename, std::ios::binary);

    if(!in.is_open()){
        throw std::runtime_error("Failed to open file for reading"); //@TODO: Implement the Try-Catch block for this later.
    }

    std::string commitId = utils::readString(in);
    std::string commitMsg = utils::readString(in);
    std::string parentCommitId = utils::readString(in);
    std::string date = utils::readString(in);
    std::string time = utils::readString(in);

    size_t mapSize;
    in.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    std::unordered_map<std::string, std::string> fileBlob;

    for(size_t i = 0; i < mapSize; i++){
        std::string key = utils::readString(in);
        std::string value = utils::readString(in);
        fileBlob[key] = value;
    }

    in.close();

    return Commit(commitId, commitMsg, parentCommitId, fileBlob, date, time);
}
