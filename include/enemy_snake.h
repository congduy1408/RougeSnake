#pragma once

#include "include/snake.h"

class EnemySnake {
    public:
        virtual ~EnemySnake() = default;
        virtual void Reset(Vector2 head_position = Vector2{30, 8},
                           direction start_direction = dir_left, int start_length = 3);
        virtual bool Update(const std::vector<bool>& wall_cells);
        virtual void Draw();
        virtual void SetTarget(Vector2 target_position, bool target_available);
        virtual void Kill();
        bool IsAlive() const;
        Snake& GetSnake();
        const Snake& GetSnake() const;
        Vector2 GetLastDeathPosition() const;
        int GetLastDeathLength() const;
        void ConfigureMovementSpeed(float base_multiplier, int boost_chance_percent,
                                    float boost_multiplier, int boost_min_moves,
                                    int boost_max_moves);

    protected:
        Snake snake;
        bool alive = false;
        double last_move_time = 0.0;
        int boost_moves_remaining = 0;
        float base_speed_multiplier = 1.0f;
        float speed_boost_multiplier = 1.5f;
        int speed_boost_chance_percent = 0;
        int speed_boost_min_moves = 3;
        int speed_boost_max_moves = 6;
        Vector2 last_death_position = {};
        int last_death_length = 0;

        bool IsMoveReady();
        virtual void UpdateDirection(const std::vector<bool>& wall_cells);
        void UpdateSpeedBoost();
        bool IsWallCell(Vector2 position, const std::vector<bool>& wall_cells) const;
        Vector2 GetNextPosition(Vector2 position, direction move_direction) const;
};
