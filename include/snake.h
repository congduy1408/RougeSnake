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
        void Reset(Vector2 head_position = Vector2{10, 15}, direction start_direction = dir_right,
                   int start_length = 3);
        void Draw();
        int TailCut(int cut_index);
        void ReadInput(bool invert_controls = false, bool allow_opposite_direction = false);
        bool QueueDirection(direction new_direction, bool allow_opposite_direction = false);
        void MoveSnake();
        // snakestate CheckSnakeState(gamestate &gamestate, food &food);
        int CheckSnakeState();
        void Grow();
        bool IsAlive() const;
        void SetMovementSpeedMultiplier(float multiplier);
        void SetSpeedBoost(bool active, float multiplier = 1.5f);
        void SetTint(Color tint);

        int Update();
    private:
        int fps = 15;
        int frame_counter = 0;
        int flip_frame = 1;
        float normal_move_interval = 0.1f;
        Color snake_tint = WHITE;
        std::vector<direction> input_queue;
        void ReverseDirection(direction new_direction);
        Color GetDrawTint() const;
        void DrawSnakePart(Rectangle draw_sprite, Rectangle draw_pos,direction dir);
};
