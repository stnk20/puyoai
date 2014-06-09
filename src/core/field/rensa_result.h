#ifndef CORE_RENSA_RESULT_H_
#define CORE_RENSA_RESULT_H_

#include <string>

#include "base/base.h"
#include "core/field/core_field.h"

struct BasicRensaResult {
    BasicRensaResult() : chains(0), score(0), frames(0) {}

    BasicRensaResult(int chains, int score, int frames) : chains(chains), score(score), frames(frames) {}

    std::string toString() const;

    friend bool operator==(const BasicRensaResult& lhs, const BasicRensaResult& rhs) {
        return lhs.chains == rhs.chains &&
            lhs.score == rhs.score &&
            lhs.frames == rhs.frames;
    }

    friend bool operator!=(const BasicRensaResult& lhs, const BasicRensaResult& rhs) {
        return !(lhs == rhs);
    }

    int chains;
    int score;
    int frames;
};

// RensaTrackResult represents in what-th chain puyo is erased.
class RensaTrackResult {
public:
    RensaTrackResult& operator=(const RensaTrackResult& result);

    // Nth Rensa where (x, y) is erased. 0 if not erased.
    int erasedAt(int x, int y) const { return m_erasedAt[x][y]; }
    void setErasedAt(int x, int y, int nthChain) { m_erasedAt[x][y] = nthChain; }

    std::string toString() const;

private:
    byte m_erasedAt[PlainField::MAP_WIDTH][PlainField::MAP_HEIGHT];
};

#endif
