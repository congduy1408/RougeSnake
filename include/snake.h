#pragma once
#include "common.h"
#include "include/gamestate.h"
#include "include/food.h"

class snake {
    public:
        std::vector<Vector2> body;
        int init_length = 3;
        direction snake_move;
        snake();
        void Draw();
        void TailCut(int cut_index);
        void ReadInput();
        void MoveSnake();
        void CheckSnakeState(gamestate &gamestate, food &food);
        void Update(gamestate &gamestate, food &food);
};