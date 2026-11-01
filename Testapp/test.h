#pragma once
#include "include/common.h"
#include "include/snake.h"

class TestApp {
    public:
        void ModifySnakeHp(Snake& snake, int index, std::vector<Soldier>& detached_soldiers);
};
