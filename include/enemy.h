#pragma once
#include "gameobject.h"
#include "board.h"

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
        virtual bool Update(const Board& board, float delta_time);
    private:
        float movement_interval = 0.10f;
        float movement_elapsed = 0.0f;
};