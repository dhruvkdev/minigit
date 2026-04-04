#include "fileUtils/hash.h"
#include <string>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>
namespace hash{
  std::string sha256(const std::string& input) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, charset.size()-1);

    std::string current = input;
    for(int i=0; i<64; i++){
      current += charset[dist(gen)];
    }
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(reinterpret_cast<const unsigned char*>(current.c_str()),
           current.size(),
           hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << (int)hash[i];
    }

    return ss.str(); // 64 hex characters
  }
  std::string generateHash(const std::string& input){
    std::string header = "blob " + std::to_string(input.size()) + '\0';
    return sha256(header + input);
  }
}
