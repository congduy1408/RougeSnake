#pragma once
#include "common.h"
#include "gameobject.h"
#include "board.h"
#include "include/snake.h"

struct EnemyAction {
    std::vector<GridPosition> attack_zone;
    int damage = 0;
};

enum class EnemyType { GraveZombie };

class Enemy: public GameObject {
    protected:
        int hp = 3;
    public:
        explicit Enemy(GridPosition position, EnemyType type = EnemyType::GraveZombie);
        EnemyType GetType() const { return type; }
        const char* GetName() const;
        virtual ~Enemy() = default;
        virtual void Draw() const;
        void TakeDamage(int damage);
        bool IsDead() const;
        void ApplySlow(float duration);
        void SetDetectionReduction(int reduction) { detection_reduction = std::max(0, reduction); }
        void OnSnakeEnter(Snake& snake) override;
        virtual EnemyAction Update(const Board& board, const Snake& snake,
                                   const std::vector<Soldier>& detached_soldiers, float delta_time);
    private:
        EnemyType type;
        float movement_interval = 0.10f;
        float movement_elapsed = 0.0f;

        float ghost_phase = 0.3f;

        int observation_radius = 1;
        int detection_reduction = 0;
        float slow_remaining = 0.0f;
        std::vector<GridPosition> visible_attack_zone;

        AttackPhase attack_phase = AttackPhase::Ready;
        float phase_elapsed = 0.0f;
        float prepare_duration = 0.5f;
        float attack_duration = 0.08f;
        float recovery_duration = 1.0f;
        // GridPosition pending_target;
        std::vector<GridPosition> pending_target;

        void ChangeAttackPhase(AttackPhase next);
        EnemyAction UpdateAttack(float delta_time);
        EnemyAction UpdateBehavior(const Board& board, const Snake& snake,
                                   const std::vector<Soldier>& detached_soldiers, float delta_time);
};
