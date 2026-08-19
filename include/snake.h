#pragma once
#include "common.h"
#include "include/gamestate.h"
#include "include/food.h"

struct snake_body {
    Vector2 position;
    direction cur_dir;
    direction pre_dir;
    bool is_turn=false;
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
        int fps = 60;
        int frame_counter = 0;
        int flip_frame = 1;
        void DrawSnakePart(Rectangle draw_sprite, Rectangle draw_pos,direction dir);
};
