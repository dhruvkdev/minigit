#include "fileUtils/diff.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace diff {

namespace {

struct Point {
  int x, y;
  bool operator<(const Point& other) const {
    return x < other.x && y < other.y;
  }
  bool operator==(const Point& other) const {
    return x == other.x && y == other.y;
  }
};

struct Snake {
  Point from;
  Point to;
};

struct EditGraphArea {
  Point top_left;
  Point bottom_right;
  int width() const { return bottom_right.x - top_left.x; }
  int height() const { return bottom_right.y - top_left.y; }
  int size() const { return width() + height(); }
  int delta() const { return width() - height(); }
};

class FurthestReaching {
  std::vector<int> v_;
public:
  explicit FurthestReaching(size_t size) : v_(size, 0) {}
  void reset() { std::fill(v_.begin(), v_.end(), 0); }
  int& operator[](int index) {
    size_t idx = index >= 0 ? index : v_.size() + index;
    return v_[idx];
  }
  const int& operator[](int index) const {
    size_t idx = index >= 0 ? index : v_.size() + index;
    return v_[idx];
  }
};

class MyersDiffer {
  FurthestReaching fr_forward_;
  FurthestReaching fr_reverse_;

public:
  MyersDiffer(size_t len1, size_t len2)
      : fr_forward_(len1 + len2 + 1),
        fr_reverse_(len1 + len2 + 1) {}

  std::optional<Snake> ShortestEditForward(const EditGraphArea& area, int d,
                                           const std::vector<std::string>& a,
                                           const std::vector<std::string>& b) {
    Point from, to;
    for (int k = -d; k <= d; k += 2) {
      if (k == -d || (k != d && fr_forward_[k - 1] < fr_forward_[k + 1])) {
        from.x = to.x = fr_forward_[k + 1];
      } else {
        from.x = fr_forward_[k - 1];
        to.x = from.x + 1;
      }
      to.y = area.top_left.y + (to.x - area.top_left.x) - k;
      from.y = (d == 0 || from.x != to.x) ? to.y : to.y - 1;

      while (to.x < area.bottom_right.x && to.y < area.bottom_right.y &&
             a[to.x] == b[to.y]) {
        ++to.x;
        ++to.y;
      }

      fr_forward_[k] = to.x;

      const bool odd = area.delta() % 2 != 0;
      const int l = k - area.delta();
      if (odd && l >= (-d + 1) && l <= d - 1 && to.x >= fr_reverse_[l]) {
        return Snake{from, to};
      }
    }
    return std::nullopt;
  }

  std::optional<Snake> ShortestEditReverse(const EditGraphArea& area, int d,
                                           const std::vector<std::string>& a,
                                           const std::vector<std::string>& b) {
    Point from, to;
    for (int l = d; l >= -d; l -= 2) {
      if (l == d || (l != -d && fr_reverse_[l - 1] > fr_reverse_[l + 1])) {
        from.x = to.x = fr_reverse_[l - 1];
      } else {
        from.x = fr_reverse_[l + 1];
        to.x = from.x - 1;
      }
      const int k = l + area.delta();
      to.y = area.top_left.y + (to.x - area.top_left.x) - k;
      from.y = (d == 0 || from.x != to.x) ? to.y : to.y + 1;

      while (to.x > area.top_left.x && to.y > area.top_left.y &&
             a[to.x - 1] == b[to.y - 1]) {
        --to.x;
        --to.y;
      }

      fr_reverse_[l] = to.x;

      const bool even = area.delta() % 2 == 0;
      if (even && k >= -d && k <= d && to.x <= fr_forward_[k]) {
        return Snake{to, from};
      }
    }
    return std::nullopt;
  }

  std::optional<Snake> FindMiddleSnake(Point from, Point to,
                                       const std::vector<std::string>& a,
                                       const std::vector<std::string>& b) {
    EditGraphArea area{from, to};
    if (area.size() == 0) return std::nullopt;

    fr_forward_.reset();
    fr_reverse_.reset();

    fr_forward_[1] = area.top_left.x;
    fr_reverse_[-1] = area.bottom_right.x;

    int maxD = static_cast<int>(std::ceil(area.size() / 2.0f));
    for (int d = 0; d <= maxD; ++d) {
      if (auto snake = ShortestEditForward(area, d, a, b)) return snake;
      if (auto snake = ShortestEditReverse(area, d, a, b)) return snake;
    }

    return std::nullopt;
  }

  std::vector<Point> FindEditPath(Point from, Point to,
                                  const std::vector<std::string>& a,
                                  const std::vector<std::string>& b) {
    if (from.x == to.x) {
      std::vector<Point> path;
      for (int y = from.y; y <= to.y; ++y) {
        path.push_back({from.x, y});
      }
      return path;
    }
    if (from.y == to.y) {
      std::vector<Point> path;
      for (int x = from.x; x <= to.x; ++x) {
        path.push_back({x, from.y});
      }
      return path;
    }

    std::optional<Snake> snake = FindMiddleSnake(from, to, a, b);
    if (!snake) {
      std::vector<Point> path;
      path.push_back(from);
      path.push_back(to);
      return path;
    }

    std::vector<Point> head = FindEditPath(from, snake->from, a, b);
    std::vector<Point> tail = FindEditPath(snake->to, to, a, b);

    std::vector<Point> result = head;
    result.insert(result.end(), tail.begin(), tail.end());
    return result;
  }
};

std::vector<Hunk> pathToHunks(const std::vector<Point>& path,
                              const std::vector<std::string>& a,
                              const std::vector<std::string>& b) {
  std::vector<Hunk> hunks;
  auto addHunk = [&](Op op, const std::string& line) {
    if (!hunks.empty() && hunks.back().op == op) {
      hunks.back().lines.push_back(line);
    } else {
      hunks.push_back({op, {line}});
    }
  };

  for (size_t i = 1; i < path.size(); ++i) {
    Point p1 = path[i - 1];
    Point p2 = path[i];

    int x = p1.x;
    int y = p1.y;
    int k1 = x - y;
    int k2 = p2.x - p2.y;

    if (k2 > k1) {
      addHunk(Op::Delete, a[x]);
      x++;
    } else if (k2 < k1) {
      addHunk(Op::Insert, b[y]);
      y++;
    }

    while (x < p2.x && y < p2.y) {
      addHunk(Op::Equal, a[x]);
      x++;
      y++;
    }
  }
  return hunks;
}

std::string joinLines(const std::vector<std::string>& lines) {
  std::ostringstream oss;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (i > 0) oss << '\n';
    oss << lines[i];
  }
  return oss.str();
}

}  // namespace

std::vector<std::string> splitLines(const std::string& text) {
  std::vector<std::string> lines;
  if (text.empty()) return lines;

  size_t start = 0;
  for (size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\n') {
      lines.emplace_back(text.substr(start, i - start));
      start = i + 1;
    }
  }
  if (start < text.size() || (!text.empty() && text.back() == '\n')) {
    lines.emplace_back(text.substr(start));
  }
  return lines;
}

std::vector<Hunk> myersDiff(const std::vector<std::string>& a,
                            const std::vector<std::string>& b) {
  if (a == b) {
    if (a.empty()) return {};
    return {{Op::Equal, a}};
  }

  MyersDiffer differ(a.size(), b.size());
  std::vector<Point> path = differ.FindEditPath(Point{0, 0}, Point{static_cast<int>(a.size()), static_cast<int>(b.size())}, a, b);
  return pathToHunks(path, a, b);
}

std::string applyHunksToBase(const std::vector<std::string>& base,
                             const std::vector<Hunk>& hunks) {
  std::vector<std::string> result;
  for (const Hunk& h : hunks) {
    switch (h.op) {
      case Op::Equal:
      case Op::Insert:
        result.insert(result.end(), h.lines.begin(), h.lines.end());
        break;
      case Op::Delete:
        break;
    }
  }
  return joinLines(result);
}

std::string formatUnified(const std::string& oldName, const std::string& newName,
                          const std::string& oldText, const std::string& newText) {
  auto oldLines = splitLines(oldText);
  auto newLines = splitLines(newText);
  auto hunks = myersDiff(oldLines, newLines);

  std::ostringstream out;
  out << "--- " << oldName << "\n";
  out << "+++ " << newName << "\n";

  size_t oldLine = 1;
  size_t newLine = 1;

  for (const Hunk& h : hunks) {
    if (h.op == Op::Equal) {
      oldLine += h.lines.size();
      newLine += h.lines.size();
      continue;
    }

    size_t oldCount = 0;
    size_t newCount = 0;
    std::ostringstream hunkBody;

    if (h.op == Op::Delete) {
      oldCount = h.lines.size();
      for (const auto& line : h.lines) hunkBody << '-' << line << '\n';
    } else if (h.op == Op::Insert) {
      newCount = h.lines.size();
      for (const auto& line : h.lines) hunkBody << '+' << line << '\n';
    }

    out << "@@ -" << oldLine << ',' << oldCount << " +" << newLine << ',' << newCount
        << " @@\n";
    out << hunkBody.str();
    oldLine += oldCount;
    newLine += newCount;
  }

  return out.str();
}

struct RegionEdit {
  size_t baseStart = 0;
  size_t baseEnd = 0;
  std::vector<std::string> replacement;
};

std::vector<RegionEdit> extractEdits(const std::vector<std::string>& baseLines,
                                     const std::vector<Hunk>& script) {
  std::vector<RegionEdit> edits;
  size_t baseIdx = 0;
  size_t hunkIdx = 0;

  while (hunkIdx < script.size()) {
    const Hunk& h = script[hunkIdx];
    if (h.op == Op::Equal) {
      baseIdx += h.lines.size();
      ++hunkIdx;
      continue;
    }

    RegionEdit edit;
    edit.baseStart = baseIdx;

    while (hunkIdx < script.size() && script[hunkIdx].op != Op::Equal) {
      const Hunk& cur = script[hunkIdx++];
      if (cur.op == Op::Delete) {
        baseIdx += cur.lines.size();
      } else {
        edit.replacement.insert(edit.replacement.end(), cur.lines.begin(),
                                cur.lines.end());
      }
    }

    edit.baseEnd = baseIdx;
    edits.push_back(std::move(edit));
  }

  (void)baseLines;
  return edits;
}

bool regionsOverlap(const RegionEdit& a, const RegionEdit& b) {
  return a.baseStart < b.baseEnd && b.baseStart < a.baseEnd;
}

MergeResult threeWayMerge(const std::string& base, const std::string& ours,
                          const std::string& theirs) {
  MergeResult result;

  if (ours == theirs) {
    result.text = ours;
    return result;
  }
  if (ours == base) {
    result.text = theirs;
    return result;
  }
  if (theirs == base) {
    result.text = ours;
    return result;
  }

  auto baseLines = splitLines(base);
  auto oursLines = splitLines(ours);
  auto theirsLines = splitLines(theirs);

  auto oursEdits = extractEdits(baseLines, myersDiff(baseLines, oursLines));
  auto theirsEdits = extractEdits(baseLines, myersDiff(baseLines, theirsLines));

  size_t oursEdit = 0;
  size_t theirsEdit = 0;
  size_t baseIdx = 0;
  bool conflict = false;
  std::vector<std::string> merged;

  while (baseIdx < baseLines.size() || oursEdit < oursEdits.size() ||
         theirsEdit < theirsEdits.size()) {
    bool hasOurs = oursEdit < oursEdits.size();
    bool hasTheirs = theirsEdit < theirsEdits.size();

    size_t nextOurs = hasOurs ? oursEdits[oursEdit].baseStart : baseLines.size();
    size_t nextTheirs =
        hasTheirs ? theirsEdits[theirsEdit].baseStart : baseLines.size();
    size_t nextEdit = std::min(nextOurs, nextTheirs);

    while (baseIdx < nextEdit) {
      merged.push_back(baseLines[baseIdx++]);
    }

    if (hasOurs && hasTheirs && nextOurs == nextTheirs) {
      const RegionEdit& o = oursEdits[oursEdit];
      const RegionEdit& t = theirsEdits[theirsEdit];
      if (regionsOverlap(o, t)) {
        if (o.replacement == t.replacement && o.baseStart == t.baseStart &&
            o.baseEnd == t.baseEnd) {
          merged.insert(merged.end(), o.replacement.begin(), o.replacement.end());
        } else {
          conflict = true;
          std::ostringstream marker;
          marker << "<<<<<<< ours\n";
          for (const auto& line : o.replacement) marker << line << '\n';
          marker << "=======\n";
          for (const auto& line : t.replacement) marker << line << '\n';
          marker << ">>>>>>> theirs\n";
          merged.push_back(marker.str());
        }
      }
      baseIdx = std::max(o.baseEnd, t.baseEnd);
      ++oursEdit;
      ++theirsEdit;
      continue;
    }

    if (hasOurs && (!hasTheirs || nextOurs <= nextTheirs)) {
      const RegionEdit& o = oursEdits[oursEdit++];
      merged.insert(merged.end(), o.replacement.begin(), o.replacement.end());
      baseIdx = o.baseEnd;
      continue;
    }

    if (hasTheirs) {
      const RegionEdit& t = theirsEdits[theirsEdit++];
      merged.insert(merged.end(), t.replacement.begin(), t.replacement.end());
      baseIdx = t.baseEnd;
    }
  }

  result.conflict = conflict;
  result.text = joinLines(merged);
  return result;
}

}  // namespace diff
