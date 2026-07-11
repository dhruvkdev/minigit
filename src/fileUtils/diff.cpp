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
    int x = path[i - 1].x;
    int y = path[i - 1].y;
    const int ex = path[i].x;
    const int ey = path[i].y;

    // A segment from (x,y) to (ex,ey) consists of:
    //   - at most one horizontal step (delete from a) or vertical step (insert from b)
    //   - followed by zero or more diagonal steps (equal)
    // OR it can be a pure-diagonal run (when from == to on the same diagonal).

    while (x != ex || y != ey) {
      if (x < ex && y < ey && x < static_cast<int>(a.size()) && y < static_cast<int>(b.size()) && a[x] == b[y]) {
        // diagonal — equal
        addHunk(Op::Equal, a[x]);
        ++x; ++y;
      } else if (x < ex && (y >= ey || (x < static_cast<int>(a.size()) && (y >= static_cast<int>(b.size()) || a[x] != b[y])))) {
        // horizontal — delete
        addHunk(Op::Delete, a[x]);
        ++x;
      } else {
        // vertical — insert
        addHunk(Op::Insert, b[y]);
        ++y;
      }
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

  // Fast paths
  if (ours == theirs) { result.text = ours; return result; }
  if (ours == base)   { result.text = theirs; return result; }
  if (theirs == base) { result.text = ours; return result; }

  auto baseLines   = splitLines(base);
  auto oursLines   = splitLines(ours);
  auto theirsLines = splitLines(theirs);

  auto oursEdits   = extractEdits(baseLines, myersDiff(baseLines, oursLines));
  auto theirsEdits = extractEdits(baseLines, myersDiff(baseLines, theirsLines));

  size_t oi = 0;          // index into oursEdits
  size_t ti = 0;          // index into theirsEdits
  size_t baseIdx = 0;     // next unconsumed base line
  bool conflict = false;
  std::vector<std::string> merged;

  while (oi < oursEdits.size() || ti < theirsEdits.size()) {

    // Determine next edit positions
    size_t nextO = (oi < oursEdits.size())   ? oursEdits[oi].baseStart   : baseLines.size();
    size_t nextT = (ti < theirsEdits.size()) ? theirsEdits[ti].baseStart : baseLines.size();

    // Case: only ours remaining
    if (ti >= theirsEdits.size()) {
      while (baseIdx < oursEdits[oi].baseStart)
        merged.push_back(baseLines[baseIdx++]);
      const RegionEdit& o = oursEdits[oi++];
      merged.insert(merged.end(), o.replacement.begin(), o.replacement.end());
      baseIdx = o.baseEnd;
      continue;
    }

    // Case: only theirs remaining
    if (oi >= oursEdits.size()) {
      while (baseIdx < theirsEdits[ti].baseStart)
        merged.push_back(baseLines[baseIdx++]);
      const RegionEdit& t = theirsEdits[ti++];
      merged.insert(merged.end(), t.replacement.begin(), t.replacement.end());
      baseIdx = t.baseEnd;
      continue;
    }

    // Check if the two next edits actually overlap
    bool overlaps = oursEdits[oi].baseStart < theirsEdits[ti].baseEnd &&
                    theirsEdits[ti].baseStart < oursEdits[oi].baseEnd;

    if (!overlaps && nextO < nextT) {
      // Ours edit comes first and doesn't touch theirs — apply independently
      while (baseIdx < oursEdits[oi].baseStart)
        merged.push_back(baseLines[baseIdx++]);
      const RegionEdit& o = oursEdits[oi++];
      merged.insert(merged.end(), o.replacement.begin(), o.replacement.end());
      baseIdx = o.baseEnd;
      continue;
    }

    if (!overlaps && nextT < nextO) {
      // Theirs edit comes first and doesn't touch ours — apply independently
      while (baseIdx < theirsEdits[ti].baseStart)
        merged.push_back(baseLines[baseIdx++]);
      const RegionEdit& t = theirsEdits[ti++];
      merged.insert(merged.end(), t.replacement.begin(), t.replacement.end());
      baseIdx = t.baseEnd;
      continue;
    }

    // Both edits start at the same position — regardless of overlap, handle together
    // (Even if they don't overlap by range, same-start means we must compare them.)

    // Gather all edits that form one overlapping conflict region
    size_t regionStart = std::min(oursEdits[oi].baseStart, theirsEdits[ti].baseStart);
    size_t regionEnd   = std::max(oursEdits[oi].baseEnd,   theirsEdits[ti].baseEnd);

    // Flush base lines up to region start
    while (baseIdx < regionStart) merged.push_back(baseLines[baseIdx++]);

    // Expand region while more edits overlap it
    size_t oi2 = oi + 1, ti2 = ti + 1;
    bool expanded = true;
    while (expanded) {
      expanded = false;
      while (oi2 < oursEdits.size() && oursEdits[oi2].baseStart < regionEnd) {
        regionEnd = std::max(regionEnd, oursEdits[oi2].baseEnd);
        ++oi2; expanded = true;
      }
      while (ti2 < theirsEdits.size() && theirsEdits[ti2].baseStart < regionEnd) {
        regionEnd = std::max(regionEnd, theirsEdits[ti2].baseEnd);
        ++ti2; expanded = true;
      }
    }

    // Build ours-side output for this region
    std::vector<std::string> oursSide;
    {
      size_t cur = regionStart;
      for (size_t k = oi; k < oi2; ++k) {
        while (cur < oursEdits[k].baseStart)
          oursSide.push_back(baseLines[cur++]);
        oursSide.insert(oursSide.end(),
          oursEdits[k].replacement.begin(), oursEdits[k].replacement.end());
        cur = oursEdits[k].baseEnd;
      }
      // trailing base lines within region (after last ours edit)
      while (cur < regionEnd) oursSide.push_back(baseLines[cur++]);
    }

    // Build theirs-side output for this region
    std::vector<std::string> theirsSide;
    {
      size_t cur = regionStart;
      for (size_t k = ti; k < ti2; ++k) {
        while (cur < theirsEdits[k].baseStart)
          theirsSide.push_back(baseLines[cur++]);
        theirsSide.insert(theirsSide.end(),
          theirsEdits[k].replacement.begin(), theirsEdits[k].replacement.end());
        cur = theirsEdits[k].baseEnd;
      }
      while (cur < regionEnd) theirsSide.push_back(baseLines[cur++]);
    }

    if (oursSide == theirsSide) {
      merged.insert(merged.end(), oursSide.begin(), oursSide.end());
    } else {
      conflict = true;
      merged.push_back("<<<<<<< ours");
      merged.insert(merged.end(), oursSide.begin(), oursSide.end());
      merged.push_back("=======");
      merged.insert(merged.end(), theirsSide.begin(), theirsSide.end());
      merged.push_back(">>>>>>> theirs");
    }

    baseIdx = regionEnd;
    oi = oi2;
    ti = ti2;
  }

  // Flush remaining base lines
  while (baseIdx < baseLines.size()) merged.push_back(baseLines[baseIdx++]);

  result.conflict = conflict;
  result.text = joinLines(merged);
  return result;
}

}  // namespace diff
