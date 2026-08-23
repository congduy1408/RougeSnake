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
        float move_interval = 0.1f;
        direction snake_move;
        Texture2D snake_sprite;
        Snake();
        ~Snake();
        Snake(const Snake&) = delete;
        Snake& operator=(const Snake&) = delete;
        void Reset();
        void Draw();
        int TailCut(int cut_index);
        void ReadInput();
        void MoveSnake();
        // snakestate CheckSnakeState(gamestate &gamestate, food &food);
        int CheckSnakeState();
        void Grow();
        void SetSpeedBoost(bool active, float multiplier = 1.5f);

        int Update();
    private:
        int fps = 15;
        int frame_counter = 0;
        int flip_frame = 1;
        float normal_move_interval = 0.1f;
        std::vector<direction> input_queue;
        void DrawSnakePart(Rectangle draw_sprite, Rectangle draw_pos,direction dir);
};
