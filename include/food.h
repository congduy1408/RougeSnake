#pragma once
#include "common.h"
#include "include/gameobject.h"

class Food: public GameObject {
    public:
        double available_time = 10;
        Food(Vector2 position) :
        GameObject(position) {};
        Food() {};
        // void SetSnake(std::vector<Vector2> _snake);
        bool CollideSnakePosition(Snake& snake);
        void SetFoodPosition(Snake& snake);
        void Draw();
        void Update();
        void OnSnakeEnter(Snake& snake) override;
};