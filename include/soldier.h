#pragma once
#include "common.h"

struct Soldier {
    static constexpr int initial_hp = 3;
    static constexpr float initial_ghost_time = 1.0f;
    GridPosition position;
    direction cur_dir;
    direction pre_dir;
    bool is_turn=false;
    int hp = initial_hp;
    float ghost_time = 0; 
};
