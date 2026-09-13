#pragma once
#include "common.h"

struct snake_body {
    GridPosition position;
    direction cur_dir;
    direction pre_dir;
    bool is_turn=false;
};

class Snake {
    public:
        std::vector<snake_body> body;
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

        void Grow();

        // void Update();
        bool UpdateMovement(float delta_time);
        void FixedUpdateAnimation(float delta_time);
        void SetMovementInterval(float interval);
    private:
        int fps = 60;

        float movement_interval = 0.10f;
        float movement_elapsed = 0.0f;
        float animation_interval = 0.3f;
        float animation_elapsed = 0.0f;

        direction queued_direction = dir_right;
        bool has_queued_direction = false;


        int frame_counter = 0;
        int flip_frame = 1;
        void DrawSnakePart(Rectangle draw_sprite, Rectangle draw_pos,direction dir);
        void MoveSnake();
        void CheckSnakeState();
};
