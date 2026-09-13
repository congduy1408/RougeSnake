#pragma once
#include "common.h"
#include "include/gameobject.h"

class Food: public GameObject {
    public:
        double available_time = 10;
        int score = 5;
        int max_score = 5;
        bool snake_inside_boundary = false;
        Food(GridPosition position) :
        GameObject(position) {};
        Food() {};
        // void SetSnake(std::vector<Vector2> _snake);
        bool CollideSnakePosition(Snake& snake);
        bool CollideWallPosition(const std::vector<bool>& wall_cells);
        bool IsInsideBoundary(GridPosition pos);
        bool UpdateBoundaryScore(GridPosition snake_pos);
        int GetScore();
        void ResetScore();
        void SetFoodPosition(Snake& snake);
        void SetFoodPosition(Snake& snake, const std::vector<bool>& wall_cells);
        void Draw();
        void Update();
        void OnSnakeEnter(Snake& snake) override;
};
