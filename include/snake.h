#pragma once
#include "common.h"

struct snake_body {
    Vector2 position;
    direction cur_dir;
    direction pre_dir;
    bool is_turn=false;
};

class Snake {
    public:
        std::vector<snake_body> body;
        std::vector<snake_body> turn_point_list;
        direction snake_move;
        Texture2D snake_sprite;

        Snake();
        ~Snake();

        Snake(const Snake&) = delete;
        Snake& operator=(const Snake&) = delete;

        void Reset();
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
