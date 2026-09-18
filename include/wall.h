#pragma once
#include "common.h"
#include "include/gameobject.h"

class Snake;

class Brick: public GameObject {
    public:
        Brick(GridPosition position) :
        GameObject(position) {};
        Brick() {};
        void SetBrickPos(GridPosition pos);
        void Draw() const;
        void Update();
        void OnSnakeEnter(Snake& snake) override;
};
