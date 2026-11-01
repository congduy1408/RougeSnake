#include "include/enemy.h"

Enemy::Enemy(GridPosition position)
    : GameObject(position)
{
}

void Enemy::Draw() const {
    DrawRectangle(
    position.x * cellsize,
    position.y * cellsize,
    cellsize,
    cellsize,
    RED
    );
    std::string hp_text = std::to_string(hp);
    DrawText(hp_text.c_str(), position.x * cellsize, position.y * cellsize - cellsize, 14, darkGreen);
    if (attack_phase == AttackPhase::Prepare) {
        for (const auto& cell : pending_target) {
            DrawRectangle(
                cell.x * cellsize,
                cell.y * cellsize,
                cellsize,
                cellsize,
                Fade(YELLOW, 0.5f)
            );
        }
    } else if (attack_phase == AttackPhase::Attack) {
        for (const auto& cell : visible_attack_zone) {
            DrawRectangle(
                cell.x * cellsize,
                cell.y * cellsize,
                cellsize,
                cellsize,
                Fade(RED, 0.5f)
            );
        }
    }

}

void Enemy::TakeDamage(int damage) {
    if (damage <= 0 || IsDead()) {
        return;
    }
    // decrease HP when take damge
    hp -= damage;
    if (hp < 0) hp = 0;
}

bool Enemy::IsDead() const {
    return hp <= 0;
}

void Enemy::OnSnakeEnter(Snake& snake) {
    // TakeDamage(1);
}

void Enemy::ChangeAttackPhase(AttackPhase next) {
    attack_phase = next;
    phase_elapsed = 0.0f;
}

EnemyAction Enemy::Update(const Board& board, const Snake& snake,
                          const std::vector<Soldier>& detached_soldiers, float delta_time) {
    if (IsDead()) {
        return {};
    }

    if (attack_phase != AttackPhase::Ready) {
        return UpdateAttack(delta_time);
    }

    return UpdateBehavior(board, snake, detached_soldiers, delta_time);
}

EnemyAction Enemy::UpdateAttack(float delta_time) {
    phase_elapsed += delta_time;

    switch (attack_phase) {
        case AttackPhase::Prepare: {
            if (phase_elapsed < prepare_duration) {
                return {};
            }

            EnemyAction action;
            action.attack_zone = pending_target;
            action.damage = 1;
            visible_attack_zone = action.attack_zone;
            ChangeAttackPhase(AttackPhase::Attack);
            return action;
        }
        case AttackPhase::Attack:
            if (phase_elapsed >= attack_duration) {
                visible_attack_zone.clear();
                pending_target.clear();
                ChangeAttackPhase(AttackPhase::End);
            }
            return {};
        case AttackPhase::End:
            if (phase_elapsed >= recovery_duration) {
                ChangeAttackPhase(AttackPhase::Ready);
            }
            return {};
        case AttackPhase::Ready:
            return {};
    }

    return {};
}

EnemyAction Enemy::UpdateBehavior(const Board& board, const Snake& snake,
                                  const std::vector<Soldier>& detached_soldiers, float delta_time) {
    movement_elapsed =
        std::min(movement_interval, movement_elapsed + delta_time);

    // Prefer attached soldiers, then inspect detached soldiers in storage order.
    GridPosition target_position;
    const auto find_target = [&](const std::vector<Soldier>& soldiers) {
        for (const auto& soldier : soldiers) {
            if (soldier.hp <= 0) continue;
            int dx = soldier.position.x - position.x;
            int dy = soldier.position.y - position.y;
            if (dx == 0 && dy == 0) continue;
            if (std::abs(dx) <= observation_radius &&
                std::abs(dy) <= observation_radius) {
                target_position = soldier.position;
                return true;
            }
        }
        return false;
    };
    bool target_found = find_target(snake.body) || find_target(detached_soldiers);
    // if player inside observe zone, attack the player direction
    if (target_found) {
        pending_target.clear();
        pending_target.push_back(target_position);
        ChangeAttackPhase(AttackPhase::Prepare);
        return {};
    }
    // if player not inside enemy observe zone move randomly
    // moving enemy
    if (movement_elapsed < movement_interval) {
        return {};
    }
    movement_elapsed -= movement_interval;
    // random move direction
    int direction = GetRandomValue(0,3);
    GridPosition next_pos = position;
    switch (direction) {
        case 0: // up
            next_pos.y -= 1;
            break;
        case 1: // right
            next_pos.x += 1;
            break;
        case 2: // down
            next_pos.y += 1;
            break;
        case 3: // left
            next_pos.x -=1;
            break;
    }
    // check for walkable
    if (board.IsWalkable(next_pos) &&
        !board.IsLethal(next_pos))
    {
        position = next_pos;
        return {};
    }
    return {};
}
