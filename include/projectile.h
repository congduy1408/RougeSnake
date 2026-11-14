#pragma once
#include "common.h"

enum class ProjectileVisual { Arrow, Fireball, IceSpike };

// A released projectile request, independent of its shooter's lifetime.
struct ProjectileSpawn {
    GridPosition position;
    GridPosition direction;
    float speed = 10.0f; // Grid steps per second, including diagonal steps.
    int max_distance = 8;
    int damage = 1;
    ProjectileVisual visual = ProjectileVisual::Arrow;
    float slow_duration = 0.0f;
};

// Game advances this arrow and resolves terrain/enemy collisions per crossed cell.
struct Projectile {
    GridPosition position;
    GridPosition direction;
    float speed;
    int remaining_steps;
    int damage;
    ProjectileVisual visual;
    float slow_duration;
    float movement_progress = 0.0f;
    bool has_moved = false;

    explicit Projectile(const ProjectileSpawn& spawn);
    void Draw() const;
};
