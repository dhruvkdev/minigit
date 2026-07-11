#include "fileUtils/serialization.hxx"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include "core/Commit.h"
#include "fileUtils/utils.h"

namespace {

constexpr uint32_t kCommitMagic = 0x4749544D;  // 'MGIT'
constexpr uint16_t kCommitVersion = 2;

bool isLikelyHexCommitId(const std::string& value) {
  if (value.size() != 64) return false;
  for (char c : value) {
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
  }
  return true;
}

}  // namespace

Commit Serialization::deserialize(const std::string& filename) {
  std::ifstream in(filename, std::ios::binary);

  if (!in.is_open()) {
    throw std::runtime_error("Failed to open file for reading: " + filename);
  }

  uint32_t maybeMagic = 0;
  in.read(reinterpret_cast<char*>(&maybeMagic), sizeof(maybeMagic));

  std::string commitId;
  std::string commitMsg;
  std::string parentCommitId;
  std::string secondParentCommitId;
  std::string date;
  std::string time;

  if (maybeMagic == kCommitMagic) {
    uint16_t version = 0;
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != kCommitVersion) {
      throw std::runtime_error("Unsupported commit format version");
    }

    commitId = utils::readString(in);
    commitMsg = utils::readString(in);
    parentCommitId = utils::readString(in);
    secondParentCommitId = utils::readString(in);
    date = utils::readString(in);
    time = utils::readString(in);
  } else {
    in.seekg(0);
    size_t firstFieldSize = 0;
    in.read(reinterpret_cast<char*>(&firstFieldSize), sizeof(firstFieldSize));
    if (!in) {
      throw std::runtime_error("Failed to read legacy commit file: " + filename);
    }

    std::string firstField(firstFieldSize, '\0');
    in.read(firstField.data(), static_cast<std::streamsize>(firstFieldSize));
    if (!in) {
      throw std::runtime_error("Failed to read legacy commit file: " + filename);
    }

    if (!isLikelyHexCommitId(firstField)) {
      throw std::runtime_error("Unsupported commit file format: " + filename);
    }

    commitId = firstField;
    commitMsg = utils::readString(in);
    parentCommitId = utils::readString(in);
    date = utils::readString(in);
    time = utils::readString(in);
  }

  size_t mapSize = 0;
  in.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

  std::unordered_map<std::string, std::string> fileBlob;
  for (size_t i = 0; i < mapSize; i++) {
    std::string key = utils::readString(in);
    std::string value = utils::readString(in);
    fileBlob[key] = value;
  }

  in.close();
  return Commit(commitId, commitMsg, parentCommitId, fileBlob, date, time,
                secondParentCommitId);
}
