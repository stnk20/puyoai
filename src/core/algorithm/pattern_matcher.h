#ifndef CORE_ALGORITHM_PATTERN_MATCHER_H_
#define CORE_ALGORITHM_PATTERN_MATCHER_H_

#include <tuple>

#include "core/puyo_color.h"
#include "core/algorithm/field_pattern.h"

class CoreField;
class FieldPattern;

struct PatternMatchResult {
    PatternMatchResult(bool matched, double score, int count, int allowedCount) :
        matched(matched), score(score), count(count), allowedCount(allowedCount) {}

    friend bool operator==(const PatternMatchResult& lhs, const PatternMatchResult& rhs)
    {
        return std::tie(lhs.matched, lhs.score, lhs.count, lhs.allowedCount) == std::tie(rhs.matched, rhs.score, rhs.count, rhs.allowedCount);
    }

    bool matched;
    double score;
    int count;
    int allowedCount;
};

class PatternMatcher {
public:
    PatternMatcher();

    // If |ignoreMustVar| is true, don't check the existence.
    PatternMatchResult match(const FieldPattern&, const CoreField&, bool ignoresMustVar = false);

    PuyoColor map(char var) const
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'];
    }

    bool isSet(char var) const
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'] != PuyoColor::WALL;
    }

private:
    PuyoColor set(char var, PuyoColor pc)
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'] = pc;
    }

    bool checkCell(char currentVar, PatternType neighborType, char neighborVar, PuyoColor neighborColor);

    PuyoColor map_[26];
};

#endif
