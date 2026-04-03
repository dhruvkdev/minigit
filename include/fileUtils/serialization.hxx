#pragma once
#include <string>
#include <unordered_map>
#include "core/Commit.h"

class Serialization{
  public:
   // Deserialize to store the object in disk, in the .mgit folder later.
   static Commit deserialize(const std::string& filename);
};
