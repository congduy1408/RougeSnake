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
    TakeDamage(1);
}

bool Enemy::Update(const Board& board,float delta_time) {
    movement_elapsed += delta_time;
    if (movement_elapsed < movement_interval) {
        return false;
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
        return true;
    }
    return false;
}
