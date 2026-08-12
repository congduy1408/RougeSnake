#pragma once
#include "common.h"
#include "include/gamestate.h"
#include "include/food.h"

struct snake_body {
    Vector2 position;
    direction cur_dir;
};

enum snakestate {
    EATFOOD,
    HITWALL,
    HITTAIL
};

class Snake {
    public:
        std::vector<snake_body> body;
        std::vector<snake_body> turn_point_list;
        int init_length = 3;
        direction snake_move;
        Texture2D snake_sprite;
        Snake();
        void Draw();
        void TailCut(int cut_index);
        void ReadInput();
        void MoveSnake();
        // snakestate CheckSnakeState(gamestate &gamestate, food &food);
        void CheckSnakeState();
        void Grow();

        void Update();
    private:
        void DrawSnakePart(Rectangle draw_sprite, Rectangle draw_pos,direction dir);
};