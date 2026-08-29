#pragma once

#include "include/elite_enemy.h"

class FirstBoss : public EliteEnemy {
    public:
        FirstBoss();
        void Reset(Vector2 head_position = Vector2{30, 8},
                   direction start_direction = dir_left, int start_length = 10) override;
};
