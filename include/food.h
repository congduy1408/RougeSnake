#pragma once
#include "common.h"
#include "include/gameobject.h"

enum class FoodType {
    Apple,
    Key
};

class Food: public GameObject {
    public:
        double available_time = 10;
        int score = 5;
        int max_score = 5;
        bool snake_inside_boundary = false;
        Food(Vector2 position) :
        GameObject(position) {};
        Food() {};
        void SetSpriteTexture(const Texture2D& sprite);
        void SetFoodType(FoodType new_type);
        FoodType GetFoodType() const;
        void Reset(FoodType new_type = FoodType::Apple);
        // void SetSnake(std::vector<Vector2> _snake);
        bool CollideSnakePosition(Snake& snake);
        bool CollideWallPosition(const std::vector<bool>& wall_cells);
        bool IsInsideBoundary(Vector2 pos);
        bool UpdateBoundaryScore(Vector2 snake_pos);
        int GetScore();
        void ResetScore();
        void SetFoodPosition(Snake& snake);
        void SetFoodPosition(Snake& snake, const std::vector<bool>& wall_cells);
        void SetFoodPosition(Snake& first_snake, Snake& second_snake, const std::vector<bool>& wall_cells);
        void Draw();
        void Update();
        void OnSnakeEnter(Snake& snake) override;
    private:
        const Texture2D* food_sprite = nullptr;
        FoodType food_type = FoodType::Apple;
        Rectangle GetSpriteSource() const;
};
