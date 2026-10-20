#pragma once
#include "gameobject.h"

class Enemy: public GameObject {
    protected:
        int hp = 3;
    public:
        explicit Enemy(GridPosition position);
        virtual ~Enemy() = default;
        virtual void Draw() const;\
        void TakeDamage(int damage);
        bool IsDead() const;
        void OnSnakeEnter(Snake& snake) override;
};