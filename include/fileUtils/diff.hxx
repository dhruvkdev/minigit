#pragma once

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string>
#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <vector>

using namespace std;
enum Operation : int8_t { EQUAL=0, INSERT=1, DELETE=2 };
inline char op2chr(Operation op) {
    switch (op) {
        case DELETE:
            return '-';
        case INSERT:
            return '+';
        case EQUAL:
            return '=';
        default:
            return '?';
    }
}

template <class String>
class Diff {
// Defaults.
    // Set these on your diff_match_patch instance to override the defaults.

    /**
     * Number of milliseconds to map a diff before giving up (0 for infinity).
     */
    long Diff_Timeout = 1000;
    /**
     * Cost of an empty edit operation in terms of edit characters.
     */
    uint16_t Diff_EditCost = 4;
    /**
     * At what point is no match declared (0.0 = perfection, 1.0 = very loose).
     */
    float Match_Threshold = 0.5f;
    /**
     * How far to search for a match (0 = exact location, 1000+ = broad match).
     * A match this many characters away from the expected location will add
     * 1.0 to the score (0.0 is a perfect match).
     */
    int Match_Distance = 1000;
    /**
     * When deleting a large block of text (over ~64 characters), how close do
     * the contents have to be to match the expected contents. (0.0 =
     * perfection, 1.0 = very loose).  Note that Match_Threshold controls how
     * closely the end points of a delete need to match.
     */
    float Patch_DeleteThreshold = 0.5f;
    /**
     * Chunk size for context length.
     */
    uint16_t Patch_Margin = 4;

   public:
    using Char = typename String::value_type;
    using Time = chrono::time_point<chrono::steady_clock>;
    using Size = typename String::size_type;
    using ConstIter = typename String::const_iterator;

    struct Range {
        ConstIter from, till;
        Range(ConstIter begin, ConstIter end) : from{begin}, till{end} {}
        explicit Range(const String& str)
            : from{str.begin()}, till{str.end()} {}
        bool operator==(Range b) const {
            if (till - from != b.till - b.from) return false;
            for (auto i = from, j = b.from; i < till; ++i, ++j)
                if (*i != *j) return false;
            return true;
        }
        Size size() const { return till - from; }
        Range substr(Size start) const {
            assert(start <= size());
            return Range{from + start, till};
        }
        Range substr(Size start, Size end) const {
            assert(end >= start);
            assert(start <= size());
            if (end >= size()) end = size();
            return Range{from + start, from + end};
        }
        Char operator[](Size idx) const {
            assert(idx < size());
            return *(from + idx);
        }
        Size find(Range b) const {
            auto at = std::search(from, till, b.from, b.till);
            return at < till ? at - from : -1;
        }
    };

    struct Diff {
        Operation operation;
        Range text;
        Diff(Operation op, Range text_range)
            : operation{op}, text{text_range} {}
        std::string str() const {
            string ret;
            ret.push_back(op2chr(operation));
            ret.push_back('\t');
            ret.append(text.from, text.till);
            return ret;
        }
    };

    using Diffs = std::vector<Diff>;

   private:
    const String& text1;
    const String& text2;
    Diffs result;

  String diff_text1(Diffs diffs) {
        Range text{};
        for (Diff aDiff : diffs) {
            if (aDiff.operation != INSERT) {
                text.append(aDiff.text);
            }
        }
        return text;
  }
  /**
     * Compute and return the destination text (all equalities and insertions).
     * @param diffs std::vector of Diff objects.
     * @return Destination text.
     */
    String diff_text2(Diffs diffs) {
        Range text{};
        for (Diff aDiff : diffs) {
            if (aDiff.operation != DELETE) {
                text.append(aDiff.text);
            }
        }
        return text;
    }

    struct Stats {
        Size equal, inserted, deleted;
        Stats() : equal{0}, inserted{0}, deleted{0} {}
    };

    Stats stats() const {
        Stats ret;
        for (const auto &i : result) {
            switch (i.operation) {
                case EQUAL:  ret.equal += i.text.size(); break;
                case INSERT: ret.inserted += i.text.size(); break;
                case DELETE: ret.deleted += i.text.size(); break;
            }
        }
        return ret;
    }
};
