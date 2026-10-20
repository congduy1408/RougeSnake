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
