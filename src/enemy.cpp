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
    if (attack_flash_remaining > 0.0f) {
        for (const auto& cell : visible_attack_zone) {
            DrawRectangle(
                cell.x * cellsize,
                cell.y * cellsize,
                cellsize,
                cellsize,
                Fade(YELLOW, 0.5f)
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

EnemyAction Enemy::Update(const Board& board, const Snake& snake, float delta_time) {
    EnemyAction action;
    attack_flash_remaining =
        std::max(0.0f, attack_flash_remaining - delta_time);

    movement_elapsed =
        std::min(movement_interval, movement_elapsed + delta_time);

    attack_elapsed =
        std::min(attack_interval, attack_elapsed + delta_time);
    // Check player enter enemy observe zone
    bool target_found = false;
    // direction attack_direction = dir_up;
    GridPosition target_position;
    for (const auto& node : snake.body) {
        int dx = node.position.x - position.x;
        int dy = node.position.y - position.y;

        if (std::abs(dx) <= observation_radius &&
                    std::abs(dy) <= observation_radius) {
            target_position = node.position;
            target_found = true;
            // if (dx == 0 && dy == 0) attack_direction = dir_up; // snake ontop of enemy, temp set attack up
            // if (dx > 0) attack_direction = dir_right;
            // if (dx < 0) attack_direction = dir_left;
            // if (dy > 0) attack_direction = dir_up;
            // if (dy < 0) attack_direction = dir_down;
            break; // future update: detect multiple snake node
        }
    }
    // if player inside observe zone, attack the player direction
    if (target_found) {
        if (attack_elapsed < attack_interval) {
            return {};
        }
        attack_elapsed = 0.0f;
        action.attack_zone.push_back(target_position);
        action.damage = 1;

        visible_attack_zone = action.attack_zone;
        attack_flash_remaining = 0.08f;

        return action;
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
    return action;
}
