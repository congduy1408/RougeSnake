#pragma once

#include "include/elite_enemy.h"

enum class FirstBossPhase {
    AttackPlayer,
    SeekFood
};

class FirstBoss : public EliteEnemy {
    public:
        int attack_length_threshold = 7;

        FirstBoss();
        void Reset(Vector2 head_position = Vector2{30, 8},
                   direction start_direction = dir_left, int start_length = 10) override;
        void SetTargets(Vector2 player_position, Vector2 food_position, bool food_available);
        FirstBossPhase GetPhase() const;

    private:
        FirstBossPhase phase = FirstBossPhase::AttackPlayer;
};
