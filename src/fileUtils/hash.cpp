#include "fileUtils/hash.h"
namespace hash{
  std::string generateHash(const std::string& input){
    return "hash_" + std::to_string(input.length() + rand()%1000);
  }
}
