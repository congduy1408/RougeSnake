#pragma once
#include "common.h"
#include "include/gamestate.h"

class food {
    public:
        Vector2 position;
        double available_time = 10;
        std::vector<Vector2> checksnake;
        food();
        void SetSnake(std::vector<Vector2> _snake);
        bool CollideSnakePosition();
        void SetFoodPosition();
        void Draw();
        void Update();
};