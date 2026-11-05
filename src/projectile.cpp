#include "include/projectile.h"

// Copy release data so an arrow survives its shooter's death or detachment.
Projectile::Projectile(const ProjectileSpawn& spawn)
    : position(spawn.position), direction(spawn.direction), speed(spawn.speed),
      remaining_steps(spawn.max_distance), damage(spawn.damage) {
}

// Draw a small arrow centered in its current grid cell.
void Projectile::Draw() const {
    Vector2 center = {(position.x + 0.5f) * cellsize, (position.y + 0.5f) * cellsize};
    Vector2 facing = Vector2Normalize({static_cast<float>(direction.x),
                                      static_cast<float>(direction.y)});
    Vector2 tip = Vector2Add(center, Vector2Scale(facing, cellsize * 0.35f));
    Vector2 tail = Vector2Subtract(center, Vector2Scale(facing, cellsize * 0.3f));
    Vector2 wing = Vector2Subtract(tip, Vector2Scale(facing, cellsize * 0.2f));
    Vector2 side = {-facing.y * cellsize * 0.15f, facing.x * cellsize * 0.15f};
    DrawLineEx(tail, tip, 2.0f, ORANGE);
    DrawLineEx(tip, Vector2Add(wing, side), 2.0f, ORANGE);
    DrawLineEx(tip, Vector2Subtract(wing, side), 2.0f, ORANGE);
}
