#include "include/soldier.h"
#include "include/enemy.h"
#include <limits>

SoldierStats GetSoldierStats(SoldierType type) {
    SoldierStats stats;
    switch (type) {
        case SoldierType::Basic:
            break;
        case SoldierType::Veteran:
            stats.can_attack_detached = true;
            break;
        case SoldierType::Archer:
            stats.attack_style = AttackStyle::Ranged;
            stats.detection_range = 6;
            stats.prepare_duration = 0.2f;
            stats.attack_duration = 0.08f;
            stats.recovery_duration = 0.7f;
            break;
    }
    return stats;
}

Soldier::Soldier(SoldierType type)
    : type(type), stats(GetSoldierStats(type)), hp(stats.max_hp) {
}

// New recruits wait even if their type supports fighting after detachment.
bool Soldier::CanAttack(bool attached) const {
    return attached || (has_joined_squad && stats.can_attack_detached);
}

GridPosition Soldier::GetMeleeCell() const {
    return {position.x + attack_direction.x * stats.attack_reach,
            position.y + attack_direction.y * stats.attack_reach};
}

SoldierAction Soldier::ReleaseAttack() {
    // Melee emits damage cells; ranged attacks only request an arrow spawn.
    switch (stats.attack_style) {
        case AttackStyle::Melee:
            strike_cell = GetMeleeCell();
            return {{strike_cell}, stats.damage};
        case AttackStyle::Ranged: {
            SoldierAction action;
            action.projectiles.push_back({position, attack_direction, stats.projectile_speed,
                                          stats.projectile_range, stats.damage});
            return action;
        }
    }
    return {};
}

void Soldier::ResetCombat() {
    attack_phase = AttackPhase::Ready;
    phase_elapsed = 0.0f;
    attack_direction = {};
    strike_cell = {};
}

void Soldier::CancelAttack() {
    if (attack_phase == AttackPhase::Prepare || attack_phase == AttackPhase::Attack) {
        attack_phase = AttackPhase::End;
        phase_elapsed = 0.0f;
    }
    attack_direction = {};
    strike_cell = {};
}

SoldierAction Soldier::UpdateCombat(const std::vector<std::unique_ptr<Enemy>>& enemies,
                                    bool attached, float delta_time) {
    if (hp <= 0) return {};
    if (attached) has_joined_squad = true;
    const bool can_attack = CanAttack(attached);
    if (!can_attack) CancelAttack();

    if (attack_phase != AttackPhase::Ready) {
        phase_elapsed += delta_time;
        switch (attack_phase) {
            case AttackPhase::Prepare:
                if (phase_elapsed >= stats.prepare_duration) {
                    attack_phase = AttackPhase::Attack;
                    phase_elapsed = 0.0f;
                    return ReleaseAttack();
                }
                break;
            case AttackPhase::Attack:
                if (phase_elapsed >= stats.attack_duration) {
                    attack_phase = AttackPhase::End;
                    phase_elapsed = 0.0f;
                    strike_cell = {};
                }
                break;
            case AttackPhase::End:
                if (phase_elapsed >= stats.recovery_duration) ResetCombat();
                break;
            case AttackPhase::Ready:
                break;
        }
        return {};
    }

    if (!can_attack || stats.damage <= 0) return {};
    if (stats.attack_style == AttackStyle::Melee && stats.attack_reach <= 0) return {};
    if (stats.attack_style == AttackStyle::Ranged &&
        (stats.projectile_speed <= 0.0f || stats.projectile_range <= 0)) return {};
    int nearest_distance = std::numeric_limits<int>::max();
    const bool moving_horizontal = cur_dir == dir_left || cur_dir == dir_right;
    GridPosition target_offset;
    bool found = false;
    for (const auto& enemy : enemies) {
        if (enemy->IsDead()) continue;
        GridPosition target = enemy->GetPosition();
        int dx = target.x - position.x;
        int dy = target.y - position.y;
        int grid_distance = std::max(std::abs(dx), std::abs(dy));
        // Detection triggers preparation even when the strike cannot reach the enemy.
        if (grid_distance == 0 || grid_distance > stats.detection_range) continue;
        GridPosition aim = {(dx > 0) - (dx < 0), (dy > 0) - (dy < 0)};
        if (stats.attack_style == AttackStyle::Ranged) {
            // Fire from either side of this soldier's local movement direction.
            if (moving_horizontal) {
                if (dy == 0) continue;
                aim.x = 0;
            } else {
                if (dx == 0) continue;
                aim.y = 0;
            }
        }
        int distance = dx * dx + dy * dy;
        if (distance < nearest_distance) {
            nearest_distance = distance;
            target_offset = aim;
            found = true;
        }
    }
    if (found) {
        attack_direction = target_offset;
        attack_phase = AttackPhase::Prepare;
        phase_elapsed = 0.0f;
    }
    return {};
}

void Soldier::DrawAttack() const {
    if (hp <= 0) return;
    if (stats.attack_style == AttackStyle::Ranged) {
        if (attack_phase == AttackPhase::Prepare || attack_phase == AttackPhase::Attack) {
            Vector2 center = {(position.x + 0.5f) * cellsize, (position.y + 0.5f) * cellsize};
            Vector2 end = {center.x + attack_direction.x * cellsize * 0.7f,
                           center.y + attack_direction.y * cellsize * 0.7f};
            DrawLineEx(center, end, attack_phase == AttackPhase::Prepare ? 1.0f : 3.0f, ORANGE);
        }
        return;
    }
    if (attack_phase == AttackPhase::Prepare) {
        GridPosition cell = GetMeleeCell();
        DrawRectangleLines(cell.x * cellsize, cell.y * cellsize, cellsize, cellsize, SKYBLUE);
    } else if (attack_phase == AttackPhase::Attack) {
        DrawRectangle(strike_cell.x * cellsize, strike_cell.y * cellsize,
                      cellsize, cellsize, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(strike_cell.x * cellsize, strike_cell.y * cellsize,
                           cellsize, cellsize, WHITE);
    }
}
