#pragma once
#include "common.h"

// A released arrow request, independent of its soldier's lifetime.
struct ProjectileSpawn {
    GridPosition position;
    GridPosition direction;
    float speed = 10.0f; // Grid steps per second, including diagonal steps.
    int max_distance = 8;
    int damage = 1;
};

// Game advances this arrow and resolves terrain/enemy collisions per crossed cell.
struct Projectile {
    GridPosition position;
    GridPosition direction;
    float speed;
    int remaining_steps;
    int damage;
    float movement_progress = 0.0f;
    bool has_moved = false;

    explicit Projectile(const ProjectileSpawn& spawn);
    void Draw() const;
};
