#pragma once

#include "common.h"

#include <cstddef>

enum class ItemEffectType {
    SpeedBoost,
    SpeedSlow,
    InvertControls,
    ReverseDirection
};

// Add future pickups by defining their effect, sprite, spawn rules, and duration here.
struct ItemDefinition {
    ItemEffectType effect = ItemEffectType::SpeedBoost;
    Rectangle sprite_source = {};
    float spawn_interval = 15.0f;
    float pickup_expire_time = 8.0f;
    int base_spawn_chance_percent = 10;
    int combo_size = 5;
    int combo_spawn_chance_percent = 5;
    float effect_duration = 5.0f;
    float effect_value = 1.5f;
    bool falls_with_rocks = false;
};

struct ActiveItem {
    std::size_t definition_index = 0;
    Vector2 position = {};
    double expire_at = 0.0;
};

struct ActiveItemEffect {
    ItemEffectType type = ItemEffectType::SpeedBoost;
    float value = 1.0f;
    double expire_at = 0.0;
    bool never_expires = false;
};
