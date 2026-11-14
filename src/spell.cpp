#include "include/game.h"
#include <limits>

namespace {
int Distance(GridPosition a, GridPosition b) {
    return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
}
Enemy* Nearest(const std::vector<std::unique_ptr<Enemy>>& enemies, GridPosition origin,
               int range, const std::vector<Enemy*>& excluded = {}) {
    Enemy* result = nullptr;
    int best = std::numeric_limits<int>::max();
    for (const auto& enemy : enemies) {
        if (enemy->IsDead() || std::find(excluded.begin(), excluded.end(), enemy.get()) != excluded.end()) continue;
        const auto cell = enemy->GetPosition();
        if (Distance(cell, origin) > range) continue;
        const int dx = cell.x - origin.x, dy = cell.y - origin.y;
        const int distance = dx * dx + dy * dy;
        if (distance < best) { best = distance; result = enemy.get(); }
    }
    return result;
}
}

GridPosition game::LeaderAim() const {
    switch (spawn_snake.snake_move) {
        case dir_up: return {0, -1};
        case dir_down: return {0, 1};
        case dir_left: return {-1, 0};
        case dir_right: return {1, 0};
    }
    return {};
}

int game::LeaderSpellDamage() const {
    return inventory.HasAura(ItemEffect::HollowNight) && !spawn_snake.body.empty() &&
           spawn_snake.body.front().hp == 1 ? 2 : 1;
}

bool game::SpellBlocksEnemy(GridPosition cell) const {
    for (const auto& spell : world_spells) {
        if (spell.effect != ItemEffect::WaterWall && spell.effect != ItemEffect::EarthPillar) continue;
        if (std::find(spell.cells.begin(), spell.cells.end(), cell) != spell.cells.end()) return true;
    }
    return false;
}

bool game::CastSpell(ItemEffect effect) {
    const GridPosition head = spawn_snake.GetHeadPosition();
    const GridPosition aim = LeaderAim();
    const auto squad_at = [&](GridPosition cell) {
        return std::any_of(spawn_snake.body.begin(), spawn_snake.body.end(),
                   [cell](const Soldier& soldier) { return soldier.position == cell; }) ||
               std::any_of(detached_soldiers.begin(), detached_soldiers.end(),
                   [cell](const Soldier& soldier) { return soldier.hp > 0 && soldier.position == cell; });
    };
    const auto safe = [&](GridPosition cell) {
        return board.GetCellType(cell) == GROUND && !SpellBlocksEnemy(cell) && !squad_at(cell) &&
            !(healing_shard_active && cell == healing_shard_position) &&
            std::none_of(ground_items.begin(), ground_items.end(), [cell](const GroundItem& item) { return item.position == cell; });
    };
    switch (effect) {
        case ItemEffect::SaintsHalo: {
            bool changed = false;
            for (auto& soldier : spawn_snake.body) {
                if (soldier.hp > 0 && !soldier.shield_charge) { soldier.shield_charge = true; changed = true; }
            }
            return changed;
        }
        case ItemEffect::IceSpike: {
            SoldierAction action;
            action.projectiles.push_back({head, aim, 20, 8, LeaderSpellDamage(), ProjectileVisual::IceSpike, 2});
            ProcessSoldierAttack(action);
            return true;
        }
        case ItemEffect::LightningChain: {
            Enemy* target = Nearest(enemies, head, 6);
            if (!target) return false;
            std::vector<Enemy*> hit;
            std::vector<GridPosition> path{head};
            while (target && hit.size() < 3) {
                const auto cell = target->GetPosition();
                path.push_back(cell);
                target->TakeDamage(LeaderSpellDamage());
                hit.push_back(target);
                target = Nearest(enemies, cell, 3, hit);
            }
            world_spells.push_back({effect, path, 0.3f});
            return true;
        }
        case ItemEffect::WaterWall: {
            const GridPosition center{head.x + aim.x * 5, head.y + aim.y * 5};
            std::vector<GridPosition> cells;
            for (int i = -1; i <= 1; ++i) {
                const GridPosition cell{center.x - aim.y * i, center.y + aim.x * i};
                if (safe(cell) && !Nearest(enemies, cell, 0)) cells.push_back(cell);
            }
            if (cells.empty()) return false;
            world_spells.push_back({effect, cells, 5});
            return true;
        }
        case ItemEffect::EarthPillar: {
            Enemy* target = Nearest(enemies, head, 6);
            if (!target || !safe(target->GetPosition())) return false;
            const auto cell = target->GetPosition();
            target->TakeDamage(LeaderSpellDamage());
            world_spells.push_back({effect, {cell}, 3});
            return true;
        }
        case ItemEffect::GraveBloom: {
            GridPosition selected;
            int count = 0;
            for (int x = 0; x < board.GetWidth(); ++x) for (int y = 0; y < board.GetHeight(); ++y) {
                const GridPosition cell{x, y};
                if (!safe(cell) || Nearest(enemies, cell, 0)) continue;
                if (std::any_of(world_spells.begin(), world_spells.end(), [cell](const WorldSpell& spell) {
                    return spell.effect == ItemEffect::GraveBloom && spell.cells.front() == cell;
                })) continue;
                if (std::none_of(spawn_snake.body.begin(), spawn_snake.body.end(), [cell](const Soldier& soldier) {
                    return Distance(cell, soldier.position) <= 3;
                })) continue;
                if (GetRandomValue(1, ++count) == 1) selected = cell;
            }
            if (!count) return false;
            world_spells.erase(std::remove_if(world_spells.begin(), world_spells.end(), [](const WorldSpell& spell) {
                return spell.effect == ItemEffect::GraveBloom;
            }), world_spells.end());
            world_spells.push_back({effect, {selected}, 20});
            return true;
        }
        default: return false;
    }
}

void game::UpdateSpellTimers(float delta_time) {
    for (auto& spell : world_spells) spell.remaining -= delta_time;
    world_spells.erase(std::remove_if(world_spells.begin(), world_spells.end(), [](const WorldSpell& spell) {
        return spell.remaining <= 0;
    }), world_spells.end());
}

void game::UpdateBloomAttacks(float delta_time) {
    for (auto& spell : world_spells) {
        if (spell.effect != ItemEffect::GraveBloom) continue;
        spell.attack_elapsed += delta_time;
        if (spell.attack_elapsed < 1) continue;
        spell.attack_elapsed = 0;
        if (Enemy* target = Nearest(enemies, spell.cells.front(), 4)) target->TakeDamage(1);
    }
}

void game::DrawSpellEffects() const {
    for (const auto& spell : world_spells) {
        if (spell.effect == ItemEffect::LightningChain) {
            for (std::size_t i = 1; i < spell.cells.size(); ++i) {
                const auto a = spell.cells[i - 1], b = spell.cells[i];
                DrawLineEx({(a.x + .5f) * cellsize, (a.y + .5f) * cellsize},
                           {(b.x + .5f) * cellsize, (b.y + .5f) * cellsize}, 3, YELLOW);
            }
            const auto center = spell.cells.front();
            DrawRectangleLines((center.x - 6) * cellsize, (center.y - 6) * cellsize, 13 * cellsize, 13 * cellsize, Fade(YELLOW, 0.4f));
        } else for (const auto cell : spell.cells) {
            const Color color = spell.effect == ItemEffect::WaterWall ? SKYBLUE :
                                spell.effect == ItemEffect::EarthPillar ? BROWN : PURPLE;
            DrawRectangle(cell.x * cellsize + 1, cell.y * cellsize + 1, cellsize - 2, cellsize - 2, Fade(color, 0.7f));
            DrawRectangleLines(cell.x * cellsize, cell.y * cellsize, cellsize, cellsize, color);
            if (spell.effect == ItemEffect::GraveBloom)
                DrawCircleV({(cell.x + .5f) * cellsize, (cell.y + .5f) * cellsize}, cellsize * .2f, GREEN);
        }
    }
}
