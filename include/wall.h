#pragma once
#include "common.h"
#include "include/gameobject.h"

class Snake;

class Brick: public GameObject {
    public:
        Brick(Vector2 position) :
        GameObject(position) {};
        Brick() {};
        void SetBrickPos(Vector2 pos);
        void Draw();
        void Update();
        void OnSnakeEnter(Snake& snake) override;
};
