#include "include/first_boss.h"

FirstBoss::FirstBoss() {
    random_turns_per_target = 0;
    random_turn_chance_percent = 0;
    obstacle_avoidance_failure_percent = 0;
}

void FirstBoss::Reset(Vector2 head_position, direction start_direction, int start_length) {
    EliteEnemy::Reset(head_position, start_direction, start_length);
    snake.SetTint(Color{255, 220, 70, 255});
}
