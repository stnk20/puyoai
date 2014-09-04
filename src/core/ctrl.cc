#include "core/ctrl.h"

#include <algorithm>
#include <map>
#include <queue>
#include <tuple>
#include <vector>

#include <glog/logging.h>

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

using namespace std;

namespace {

// Remove redundant key stroke.
void removeRedundantKeySeq(const KumipuyoPos& pos, KeySetSeq* seq)
{
    switch (pos.r) {
    case 0:
        return;
    case 1:
        if (seq->size() >= 2 && (*seq)[0] == KeySet(Key::LEFT_TURN) && (*seq)[1] == KeySet(Key::RIGHT_TURN)) {
            // Remove 2 key strokes.
            seq->removeFront();
            seq->removeFront();
        }
        return;
    case 2:
        return;
    case 3:
        if (seq->size() >= 2 && (*seq)[0] == KeySet(Key::RIGHT_TURN) && (*seq)[1] == KeySet(Key::LEFT_TURN)) {
            seq->removeFront();
            seq->removeFront();
        }
        return;
    default:
        CHECK(false) << "Unknown r: " << pos.r;
        return;
    }
}

}

void Ctrl::moveKumipuyoByArrowKey(const PlainField& field, const KeySet& keySet, MovingKumipuyoState* mks, bool* downAccepted)
{
    DCHECK(mks);
    DCHECK(!mks->grounded) << "Grounded puyo cannot be moved.";

    // Only one key will be accepted.
    // When DOWN + RIGHT or DOWN + LEFT are simultaneously input, DOWN should be ignored.

    if (keySet.hasKey(Key::RIGHT)) {
        if (field.get(mks->pos.axisX() + 1, mks->pos.axisY()) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX() + 1, mks->pos.childY()) == PuyoColor::EMPTY) {
            mks->pos.x++;
        }
        return;
    }

    if (keySet.hasKey(Key::LEFT)) {
        if (field.get(mks->pos.axisX() - 1, mks->pos.axisY()) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX() - 1, mks->pos.childY()) == PuyoColor::EMPTY) {
            mks->pos.x--;
        }
        return;
    }

    if (keySet.hasKey(Key::DOWN)) {
        if (downAccepted)
            *downAccepted = true;
        if (mks->restFramesForFreefall > 0) {
            mks->restFramesForFreefall = 0;
            return;
        }

        mks->restFramesForFreefall = 0;
        if (field.get(mks->pos.axisX(), mks->pos.axisY() - 1) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX(), mks->pos.childY() - 1) == PuyoColor::EMPTY) {
            mks->pos.y--;
            return;
        }

        // Grounded.
        mks->grounded = true;
        return;
    }
}

void Ctrl::moveKumipuyoByTurnKey(const PlainField& field, const KeySet& keySet, MovingKumipuyoState* mks)
{
    DCHECK_EQ(0, mks->restFramesTurnProhibited) << mks->restFramesTurnProhibited;

    if (keySet.hasKey(Key::RIGHT_TURN)) {
        mks->restFramesTurnProhibited = FRAMES_CONTINUOUS_TURN_PROHIBITED;
        switch (mks->pos.r) {
        case 0:
            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->pos.x--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 2;
                mks->pos.y++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 1:
            if (field.get(mks->pos.x, mks->pos.y - 1) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                return;
            }

            mks->pos.r = (mks->pos.r + 1) % 4;
            mks->pos.y++;
            return;
        case 2:
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->pos.x++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 0;
                mks->pos.y--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 3:
            mks->pos.r = (mks->pos.r + 1) % 4;
            return;
        default:
            CHECK(false) << mks->pos.r;
            return;
        }
    }

    if (keySet.hasKey(Key::LEFT_TURN)) {
        mks->restFramesTurnProhibited = FRAMES_CONTINUOUS_TURN_PROHIBITED;
        switch (mks->pos.r) {
        case 0:
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->pos.x++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 2;
                mks->pos.y++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 1:
            mks->pos.r = (mks->pos.r + 3) % 4;
            return;
        case 2:
            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->pos.x--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 0;
                mks->pos.y--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 3:
            if (field.get(mks->pos.x, mks->pos.y - 1) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                return;
            }

            mks->pos.r = (mks->pos.r + 3) % 4;
            mks->pos.y++;
            return;
        default:
            CHECK(false) << mks->pos.r;
            return;
        }
    }
}

void Ctrl::moveKumipuyoByFreefall(const PlainField& field, MovingKumipuyoState* mks)
{
    DCHECK(!mks->grounded);

    if (mks->restFramesForFreefall > 1) {
        mks->restFramesForFreefall--;
        return;
    }

    mks->restFramesForFreefall = FRAMES_FREE_FALL;
    if (field.get(mks->pos.axisX(), mks->pos.axisY() - 1) == PuyoColor::EMPTY &&
        field.get(mks->pos.childX(), mks->pos.childY() - 1) == PuyoColor::EMPTY) {
        mks->pos.y--;
        return;
    }

    mks->grounded = true;
    return;
}

struct Vertex {
    Vertex() {}
    Vertex(const KumipuyoPos& pos, int framesTurnProhibited) :
        pos(pos), framesTurnProhibited(framesTurnProhibited) {}

    friend bool operator==(const Vertex& lhs, const Vertex& rhs)
    {
        return lhs.pos == rhs.pos && lhs.framesTurnProhibited == rhs.framesTurnProhibited;
    }
    friend bool operator!=(const Vertex& lhs, const Vertex& rhs) { return !(lhs.pos == rhs.pos); }
    friend bool operator<(const Vertex& lhs, const Vertex& rhs)
    {
        if (lhs.pos != rhs.pos)
            return lhs.pos < rhs.pos;
        return lhs.framesTurnProhibited < rhs.framesTurnProhibited;
    }
    friend bool operator>(const Vertex& lhs, const Vertex& rhs) { return lhs.pos > rhs.pos; }

    KumipuyoPos pos;
    int framesTurnProhibited;
};

typedef double Weight;
struct Edge {
    Edge() {}
    Edge(const Vertex& src, const Vertex& dest, Weight weight, const KeySet& keySet) :
        src(src), dest(dest), weight(weight), keySet(keySet) {}

    friend bool operator>(const Edge& lhs, const Edge& rhs) {
        if (lhs.weight != rhs.weight) { return lhs.weight > rhs.weight; }
        if (lhs.src != rhs.src) { lhs.src > rhs.src; }
        return lhs.dest > rhs.dest;
    }

    Vertex src;
    Vertex dest;
    Weight weight;
    KeySet keySet;
};

typedef vector<Edge> Edges;
typedef map<Vertex, tuple<Vertex, KeySet, Weight>> Potential;

KeySetSeq Ctrl::findKeyStrokeByDijkstra(const PlainField& field, const MovingKumipuyoState& initialState, const Decision& decision)
{
    // We don't add KeySet(Key::DOWN) intentionally.
    static const pair<KeySet, double> KEY_CANDIDATES[] = {
        make_pair(KeySet(), 1),
        make_pair(KeySet(Key::LEFT), 1.01),
        make_pair(KeySet(Key::RIGHT), 1.01),
        make_pair(KeySet(Key::LEFT, Key::LEFT_TURN), 1.03),
        make_pair(KeySet(Key::LEFT, Key::RIGHT_TURN), 1.03),
        make_pair(KeySet(Key::RIGHT, Key::LEFT_TURN), 1.03),
        make_pair(KeySet(Key::RIGHT, Key::RIGHT_TURN), 1.03),
        make_pair(KeySet(Key::LEFT_TURN), 1.01),
        make_pair(KeySet(Key::RIGHT_TURN), 1.01),
    };
    static const int KEY_CANDIDATES_WITHOUT_TURN_SIZE = 3;
    static const int KEY_CANDIDATES_SIZE = ARRAY_SIZE(KEY_CANDIDATES);

    Potential pot;
    priority_queue<Edge, Edges, greater<Edge> > Q;

    Vertex startV(initialState.pos, false);
    Q.push(Edge(startV, startV, 0, KeySet()));

    while (!Q.empty()) {
        Edge edge = Q.top(); Q.pop();
        Vertex p = edge.dest;
        Weight curr = edge.weight;

        // already visited?
        if (pot.count(p))
            continue;

        pot[p] = make_tuple(edge.src, edge.keySet, curr);

        // goal.
        if (p.pos.axisX() == decision.x && p.pos.rot() == decision.r) {
            // goal.
            vector<KeySet> kss;
            kss.push_back(KeySet(Key::DOWN));
            while (pot.count(p)) {
                if (p == startV)
                    break;
                const KeySet ks(std::get<1>(pot[p]));
                kss.push_back(ks);
                p = std::get<0>(pot[p]);
            }

            reverse(kss.begin(), kss.end());
            return KeySetSeq(kss);
        }

        int size = p.framesTurnProhibited > 0 ? KEY_CANDIDATES_WITHOUT_TURN_SIZE : KEY_CANDIDATES_SIZE;
        for (int i = 0; i < size; ++i) {
            MovingKumipuyoState mks(p.pos);
            moveKumipuyo(field, KEY_CANDIDATES[i].first, &mks);

            int framesTurnProhibited = std::max(0, p.framesTurnProhibited - 1);
            if (i >= KEY_CANDIDATES_WITHOUT_TURN_SIZE)
                framesTurnProhibited = FRAMES_CONTINUOUS_TURN_PROHIBITED;

            Vertex newP(mks.pos, framesTurnProhibited);

            if (pot.count(newP))
                continue;
            // TODO(mayah): This is not correct. We'd like to prefer KeySet() to another key sequence a bit.
            Q.push(Edge(p, newP, curr + KEY_CANDIDATES[i].second, KEY_CANDIDATES[i].first));
        }
    }

    // No way...
    return KeySetSeq();
}

void Ctrl::moveKumipuyo(const PlainField& field, const KeySet& keySet, MovingKumipuyoState* mks, bool* downAccepted)
{
    // TODO(mayah): Which key is consumed first? turn? arrow?
    moveKumipuyoByArrowKey(field, keySet, mks, downAccepted);
    if (mks->restFramesTurnProhibited > 0) {
        mks->restFramesTurnProhibited--;
    } else {
        moveKumipuyoByTurnKey(field, keySet, mks);
    }
    if (!keySet.hasKey(Key::DOWN))
        moveKumipuyoByFreefall(field, mks);
}

bool Ctrl::isReachable(const PlainField& field, const Decision& decision)
{
    if (isReachableFastpath(field, decision))
        return true;

    // slowpath
    return isReachableOnline(field, KumipuyoPos(decision.x, 1, decision.r), KumipuyoPos::initialPos());
}

bool Ctrl::isReachableFastpath(const PlainField& field, const Decision& decision)
{
    DCHECK(decision.isValid()) << decision.toString();

    static const int checker[6][4] = {
        { 2, 1, 0 },
        { 2, 0 },
        { 0 },
        { 4, 0 },
        { 4, 5, 0 },
        { 4, 5, 6, 0 },
    };

    int checkerIdx = decision.x - 1;
    if (decision.r == 1 && 3 <= decision.x)
        checkerIdx += 1;
    else if (decision.r == 3 && decision.x <= 3)
        checkerIdx -= 1;

    // When decision is valid, this should hold.
    DCHECK(0 <= checkerIdx && checkerIdx < 6) << checkerIdx;

    for (int i = 0; checker[checkerIdx][i] != 0; ++i) {
        if (field.get(checker[checkerIdx][i], 12) != PuyoColor::EMPTY)
            return false;
    }

    return true;
}

bool Ctrl::isReachableOnline(const PlainField& field, const KumipuyoPos& goal, const KumipuyoPos& start)
{
    KeySetSeq keySetSeq;
    return getControlOnline(field, goal, start, &keySetSeq);
}

bool Ctrl::isQuickturn(const PlainField& field, const KumipuyoPos& k)
{
    // assume that k.r == 0 or 2
    return (field.get(k.x - 1, k.y) != PuyoColor::EMPTY && field.get(k.x + 1, k.y) != PuyoColor::EMPTY);
}

bool Ctrl::getControl(const PlainField& field, const Decision& decision, KeySetSeq* ret)
{
    ret->clear();

    if (!isReachable(field, decision)) {
        return false;
    }
    int x = decision.x;
    int r = decision.r;

    switch (r) {
    case 0:
        moveHorizontally(x - 3, ret);
        break;

    case 1:
        add(Key::RIGHT_TURN, ret);
        moveHorizontally(x - 3, ret);
        break;

    case 2:
        moveHorizontally(x - 3, ret);
        if (x < 3) {
            add(Key::RIGHT_TURN, ret);
            add(Key::RIGHT_TURN, ret);
        } else if (x > 3) {
            add(Key::LEFT_TURN, ret);
            add(Key::LEFT_TURN, ret);
        } else {
            if (field.get(4, 12) != PuyoColor::EMPTY) {
                if (field.get(2, 12) != PuyoColor::EMPTY) {
                    // fever's quick turn
                    add(Key::RIGHT_TURN, ret);
                    add(Key::RIGHT_TURN, ret);
                } else {
                    add(Key::LEFT_TURN, ret);
                    add(Key::LEFT_TURN, ret);
                }
            } else {
                add(Key::RIGHT_TURN, ret);
                add(Key::RIGHT_TURN, ret);
            }
        }
        break;

    case 3:
        add(Key::LEFT_TURN, ret);
        moveHorizontally(x - 3, ret);
        break;

    default:
        LOG(FATAL) << r;
    }

    add(Key::DOWN, ret);

    removeRedundantKeySeq(KumipuyoPos::initialPos(), ret);

    return true;
}

// returns null if not reachable
bool Ctrl::getControlOnline(const PlainField& field, const KumipuyoPos& goal, const KumipuyoPos& start, KeySetSeq* ret)
{
    KumipuyoPos current = start;

    ret->clear();
    while (true) {
        if (goal.x == current.x && goal.r == current.r) {
            break;
        }

        // for simpicity, direct child-puyo upwards
        // TODO(yamaguchi): eliminate unnecessary moves
        if (current.r == 1) {
            add(Key::LEFT_TURN, ret);
            current.r = 0;
        } else if (current.r == 3) {
            add(Key::RIGHT_TURN, ret);
            current.r = 0;
        } else if (current.r == 2) {
            if (isQuickturn(field, current)) {
                // do quick turn
                add(Key::RIGHT_TURN, ret);
                add(Key::RIGHT_TURN, ret);
                current.y++;
            } else {
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    add(Key::LEFT_TURN, ret);
                    add(Key::LEFT_TURN, ret);
                } else {
                    add(Key::RIGHT_TURN, ret);
                    add(Key::RIGHT_TURN, ret);
                }
            }
            current.r = 0;
        }
        if (goal.x == current.x) {
            switch(goal.r) {
            case 0:
                break;
            case 1:
                if (field.get(current.x + 1, current.y) != PuyoColor::EMPTY) {
                    if (field.get(current.x + 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.get(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return false;
                    }
                    // turn inversely to avoid kicking wall
                    add(Key::LEFT_TURN, ret);
                    add(Key::LEFT_TURN, ret);
                    add(Key::LEFT_TURN, ret);
                } else {
                    add(Key::RIGHT_TURN, ret);
                }
                break;
            case 3:
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    if (field.get(current.x - 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.get(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return false;
                    }
                    add(Key::RIGHT_TURN, ret);
                    add(Key::RIGHT_TURN, ret);
                    add(Key::RIGHT_TURN, ret);
                } else {
                    add(Key::LEFT_TURN, ret);
                }
                break;
            case 2:
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    add(Key::RIGHT_TURN, ret);
                    add(Key::RIGHT_TURN, ret);
                } else {
                    add(Key::LEFT_TURN, ret);
                    add(Key::LEFT_TURN, ret);
                }
                break;
            }
            break;
        }

        // direction to move horizontally
        if (goal.x > current.x) {
            // move to right
            if (field.get(current.x + 1, current.y) == PuyoColor::EMPTY) {
                add(Key::RIGHT, ret);
                current.x++;
            } else {  // hits a wall
                // climb if possible
                /*
                  aBb
                  .A@
                  .@@.
                */
                // pivot puyo cannot go up anymore
                if (current.y >= 13) return false;
                // check "b"
                if (field.get(current.x + 1, current.y + 1) != PuyoColor::EMPTY) {
                    return false;
                }
                if (field.get(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because right side is blocked

                    add(Key::LEFT_TURN, ret);
                    add(Key::LEFT_TURN, ret);
                    current.y++;
                    if (!field.get(current.x - 1, current.y + 1)) {
                        add(Key::RIGHT_TURN, ret);
                        add(Key::RIGHT, ret);
                    } else {
                        // if "a" in the figure is filled, kicks wall. we can omit right key.
                        add(Key::RIGHT_TURN, ret);
                    }
                    add(Key::RIGHT_TURN, ret);
                    current.x++;
                } else {
                    return false;
                }
            }
        } else {
            // move to left
            if (!field.get(current.x - 1, current.y)) {
                add(Key::LEFT, ret);
                current.x--;
            } else {  // hits a wall
                // climb if possible
                /*
                  bBa
                  @A.
                  @@@.
                */
                // pivot puyo cannot go up anymore
                if (current.y >= 13) return false;
                // check "b"
                if (field.get(current.x - 1, current.y + 1) != PuyoColor::EMPTY) {
                    return false;
                }
                if (field.get(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because left side is blocked
                    add(Key::RIGHT_TURN, ret);
                    add(Key::RIGHT_TURN, ret);
                    current.y++;
                    if (!field.get(current.x + 1, current.y)) {
                        add(Key::LEFT_TURN, ret);
                        add(Key::LEFT, ret);
                    } else {
                        // if "a" in the figure is filled, kicks wall. we can omit left key.
                        add(Key::LEFT_TURN, ret);
                    }
                    add(Key::LEFT_TURN, ret);
                    current.x--;
                } else {
                    return false;
                }
            }
        }
    }

    add(Key::DOWN, ret);
    removeRedundantKeySeq(start, ret);
    // LOG(INFO) << buttonsDebugString();
    return true;
}

void Ctrl::add(Key b, KeySetSeq* ret)
{
    ret->add(KeySet(b));
}

void Ctrl::moveHorizontally(int x, KeySetSeq* ret)
{
    if (x < 0) {
        for (int i = 0; i < -x; i++) {
            add(Key::LEFT, ret);
        }
    } else if (x > 0) {
        for (int i = 0; i < x; i++) {
            add(Key::RIGHT, ret);
        }
    }
}
