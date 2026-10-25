#pragma once
#include "common.h"
#include "gameobject.h"
#include "board.h"
#include "include/snake.h"

struct EnemyAction {
    std::vector<GridPosition> attack_zone;
    int damage = 0;
};

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
        virtual EnemyAction Update(const Board& board, const Snake& snake, float delta_time);
    private:
        float movement_interval = 0.10f;
        float movement_elapsed = 0.0f;

        float attack_interval = 0.1f;
        float attack_elapsed = 0.0f;

        float ghost_phase = 0.3f;

        int observation_radius = 1;
        std::vector<GridPosition> visible_attack_zone;
        float attack_flash_remaining = 0.0f;
};