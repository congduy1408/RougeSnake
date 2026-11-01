#pragma once
#include "common.h"
#include "game_textures.h"
#include "room.h"
#include "soldier.h"


class Snake {
    public:
        std::vector<Soldier> body;
        direction snake_move;

        Snake();
        ~Snake();

        Snake(const Snake&) = delete;
        Snake& operator=(const Snake&) = delete;

        void Reset(Room& room);
        void EnterRoom(Room& room);
        void Draw(const GameTextures& textures);
        std::vector<Soldier> DetachSoldier(int dead_index);
        void ReadInput();

        void Grow();
        bool AttachSoldier(const Soldier& soldier);

        // void Update();
        bool UpdateMovement(float delta_time);
        void FixedUpdateAnimation(float delta_time);
        void SetMovementInterval(float interval);
        GridPosition GetHeadPosition() const;
        std::vector<Soldier> DamageNode(GridPosition position, int damage);
        bool IsDead() const;
        void ReduceGhostTime(float delta_time);
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

        int unfold_steps = 0;
        int init_length = 3;

        void DrawSnakePart(const Texture2D& texture, Rectangle draw_sprite, Rectangle draw_pos,direction dir, Color tint);
        void MoveSnake();
        
        bool IsUnfolding();
        bool IsInvicible();
};
