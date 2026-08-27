#pragma once

#include "include/snake.h"

class EnemySnake {
    public:
        void Reset(Vector2 head_position = Vector2{30, 8}, direction start_direction = dir_left);
        bool Update(const std::vector<bool>& wall_cells);
        void Draw();
        void Kill();
        bool IsAlive() const;
        Snake& GetSnake();
        const Snake& GetSnake() const;

    private:
        Snake snake;
        bool alive = false;
        double last_move_time = 0.0;
        int boost_moves_remaining = 0;

        bool IsMoveReady();
        void UpdateDirection(const std::vector<bool>& wall_cells);
        void UpdateSpeedBoost();
        bool IsWallCell(Vector2 position, const std::vector<bool>& wall_cells) const;
        Vector2 GetNextPosition(Vector2 position, direction move_direction) const;
};
