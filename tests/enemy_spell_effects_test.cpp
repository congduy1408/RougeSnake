#include "include/common.h"
#include "include/snake.h"
#include "include/board.h"
// Expose only enemy internals in this standalone timing/detection test.
#define private public
#include "include/enemy.h"
#undef private
#include <cassert>

int main() {
    Board board(40, 30);
    for (int x = 0; x < board.GetWidth(); ++x)
        for (int y = 0; y < board.GetHeight(); ++y) board.SetCell({x, y}, GROUND);
    Snake squad;
    squad.body.emplace_back();
    squad.body[0].position = {30, 20};
    std::vector<Soldier> waiting;
    Enemy slow({10, 10});
    slow.ApplySlow(2);
    slow.Update(board, squad, waiting, .05f);
    assert(slow.movement_elapsed == .025f && slow.slow_remaining > 1.9f);
    slow.attack_phase = AttackPhase::Prepare;
    slow.phase_elapsed = 0;
    slow.Update(board, squad, waiting, .1f);
    assert(slow.phase_elapsed == .1f); // Slow never changes attack preparation time.
    Enemy watcher({10, 10});
    watcher.observation_radius = 3;
    watcher.SetDetectionReduction(1);
    squad.body[0].position = {13, 10};
    watcher.Update(board, squad, waiting, 0);
    assert(watcher.attack_phase == AttackPhase::Ready);
    watcher.SetDetectionReduction(0);
    watcher.Update(board, squad, waiting, 0);
    assert(watcher.attack_phase == AttackPhase::Prepare);
    Enemy adjacent({10, 10});
    adjacent.SetDetectionReduction(10);
    squad.body[0].position = {11, 10};
    adjacent.Update(board, squad, waiting, 0);
    assert(adjacent.attack_phase == AttackPhase::Prepare); // Minimum radius remains one.
}
