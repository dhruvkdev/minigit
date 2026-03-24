#include "fileUtils/hash.h"
#include <string>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <string>
namespace hash{
  std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()),
           input.size(),
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
