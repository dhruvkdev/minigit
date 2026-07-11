#pragma once

#include <string>
#include <vector>

namespace diff {

enum class Op { Equal, Insert, Delete };

struct Hunk {
  Op op;
  std::vector<std::string> lines;
};

struct MergeResult {
  bool conflict = false;
  std::string text;
};

std::vector<std::string> splitLines(const std::string& text);

// Myers O(ND) diff on line sequences (same algorithm family Git uses).
std::vector<Hunk> myersDiff(const std::vector<std::string>& a,
                            const std::vector<std::string>& b);

std::string formatUnified(const std::string& oldName, const std::string& newName,
                          const std::string& oldText, const std::string& newText);

MergeResult threeWayMerge(const std::string& base, const std::string& ours,
                          const std::string& theirs);

std::string applyHunksToBase(const std::vector<std::string>& base,
                             const std::vector<Hunk>& hunks);

}  // namespace diff
