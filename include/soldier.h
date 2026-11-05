#pragma once
#include "common.h"
#include "projectile.h"

class Enemy;

enum class SoldierType { Basic, Veteran, Archer };
enum class AttackStyle { Melee, Ranged };

struct SoldierStats {
    int max_hp = 3;
    int detection_range = 2;
    int attack_reach = 1;
    int damage = 1;
    float prepare_duration = 0.1f;
    float attack_duration = 0.5f;
    float recovery_duration = 0.5f;
    bool can_attack_detached = false;
    AttackStyle attack_style = AttackStyle::Melee;
    float projectile_speed = 10.0f;
    int projectile_range = 8;
};

SoldierStats GetSoldierStats(SoldierType type);

struct SoldierAction {
    std::vector<GridPosition> attack_zone;
    int damage = 0;
    std::vector<ProjectileSpawn> projectiles;
};

struct Soldier {
    static constexpr float initial_ghost_time = 1.0f;
    SoldierType type;
    SoldierStats stats;
    std::string character_id; // Optional story identity, independent of combat type.
    GridPosition position;
    direction cur_dir = dir_right;
    direction pre_dir = dir_right;
    bool is_turn=false;
    int hp;
    float ghost_time = 0; 
    bool has_joined_squad = false;

    AttackPhase attack_phase = AttackPhase::Ready;
    float phase_elapsed = 0.0f;
    GridPosition attack_direction; // Melee: eight-way; ranged: perpendicular to local movement at aim time.
    GridPosition strike_cell;

    explicit Soldier(SoldierType type = SoldierType::Basic);
    bool CanAttack(bool attached) const;
    SoldierAction UpdateCombat(const std::vector<std::unique_ptr<Enemy>>& enemies,
                               bool attached, float delta_time);
    void CancelAttack();
    void ResetCombat();
    void DrawAttack() const;
    GridPosition GetMeleeCell() const;
    SoldierAction ReleaseAttack();
};
