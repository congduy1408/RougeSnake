#pragma once
#include "common.h"

enum class ItemCategory { Filler, Weapon, Spell, Aura };
enum class ItemEffect { None, TestSignal };

struct Item {
    std::string id;
    std::string name;
    ItemCategory category = ItemCategory::Filler;
    char letter = '?';
    std::string description;
    ItemEffect effect = ItemEffect::None;
    float cooldown = 0.0f;
    float cooldown_remaining = 0.0f;
};

const std::vector<Item>& GetFillerItems();

struct GroundItem {
    Item item;
    GridPosition position;
    bool waiting_for_departure = false;
};

struct LeaderInventory {
    std::string cipher = "STAR";
    std::vector<Item> items;
    std::size_t selected = 0;

    std::size_t Capacity() const { return cipher.size() + 2; }
    void Cycle(int step);
    void UpdateCooldowns(float delta_time);
    bool UseSelected();
    std::vector<bool> CipherMatches() const;
};
