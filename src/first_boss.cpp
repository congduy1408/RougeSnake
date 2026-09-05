#include "include/first_boss.h"

FirstBoss::FirstBoss() {
    random_turns_per_target = -1;
    random_turn_chance_percent = 20;
    obstacle_avoidance_failure_percent = 0;
}

void FirstBoss::Reset(Vector2 head_position, direction start_direction, int start_length) {
    EliteEnemy::Reset(head_position, start_direction, start_length);
    snake.SetTint(Color{255, 220, 70, 255});
    phase = FirstBossPhase::AttackPlayer;
}

void FirstBoss::SetTargets(Vector2 player_position, Vector2 food_position, bool food_available) {
    int threshold = attack_length_threshold < 3 ? 3 : attack_length_threshold;
    phase = snake.body.size() >= static_cast<size_t>(threshold)
        ? FirstBossPhase::AttackPlayer
        : FirstBossPhase::SeekFood;

    if (phase == FirstBossPhase::AttackPlayer) {
        SetAvoidPosition(Vector2{}, false);
        SetTarget(player_position, true);
        return;
    }

    SetAvoidPosition(player_position, true);
    SetTarget(food_position, food_available);
}

FirstBossPhase FirstBoss::GetPhase() const {
    return phase;
}
