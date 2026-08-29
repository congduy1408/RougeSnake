#pragma once

#include "include/enemy_snake.h"

enum class EliteEnemyState {
    DetectFood,
    FollowRoute
};

class EliteEnemy : public EnemySnake {
    public:
        int random_turns_per_target = 2;
        int random_turn_chance_percent = 15;
        int obstacle_avoidance_failure_percent = 5;

        void Reset(Vector2 head_position = Vector2{30, 8},
                   direction start_direction = dir_left, int start_length = 5) override;
        void SetTarget(Vector2 target_position, bool target_available) override;
        EliteEnemyState GetState() const;

    protected:
        void UpdateDirection(const std::vector<bool>& wall_cells) override;

    private:
        EliteEnemyState state = EliteEnemyState::DetectFood;
        Vector2 target_position = {};
        bool target_available = false;
        int random_turns_remaining = 0;
        std::vector<direction> route;
        size_t route_index = 0;

        bool PlanRoute(const std::vector<bool>& wall_cells);
        bool TryRandomTurn(const std::vector<bool>& wall_cells);
        bool IsOwnBodyCell(Vector2 position) const;
        bool IsOppositeDirection(direction first, direction second) const;
};
