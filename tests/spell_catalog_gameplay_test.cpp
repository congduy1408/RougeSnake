// Standalone integration harness. Compile with production objects except main.o.
#include "include/common.h"
#include "include/snake.h"
#include "include/gamestate.h"
#include "include/board.h"
#include "include/room_template.h"
#include "include/room.h"
#include "include/game_textures.h"
#include "include/enemy.h"
#include "include/item.h"
#define private public
#include "include/game.h"
#undef private
#include <cassert>
#include <set>

struct SpellTarget : Enemy {
    using Enemy::Enemy;
    int HP() const { return hp; }
};

int main() {
    RoomCatalog catalog;
    assert(catalog.LoadAll("map/map_manifest.json"));
    game g(catalog);
    assert(g.InitGameObject());
    g.state.currentScreen = STAGE;
    g.enemies.clear();
    for (int x = 0; x < g.board.GetWidth(); ++x)
        for (int y = 0; y < g.board.GetHeight(); ++y) g.board.SetCell({x, y}, GROUND);
    g.spawn_snake.UpdateMovement(1);
    g.spawn_snake.UpdateMovement(1); // Finish entry protection.
    for (std::size_t i = 0; i < g.spawn_snake.body.size(); ++i)
        g.spawn_snake.body[i].position = {10, 10 + static_cast<int>(i)};
    g.spawn_snake.snake_move = dir_right;
    assert(GetSpellItems().size() == 7 && GetAuraItems().size() == 3);
    std::set<std::string> ids;
    for (const auto& item : GetSpellItems()) assert(ids.insert(item.id).second && item.category == ItemCategory::Spell);
    for (const auto& item : GetAuraItems()) assert(ids.insert(item.id).second && item.category == ItemCategory::Aura);
    auto equip = [&](ItemEffect effect) {
        for (const auto& item : GetSpellItems()) if (item.effect == effect) {
            g.inventory.items = {item}; g.inventory.selected = 0; return;
        }
        assert(false);
    };
    equip(ItemEffect::SaintsHalo);
    g.UseLeaderItem();
    assert(g.inventory.items[0].cooldown_remaining == 180);
    for (const auto& soldier : g.spawn_snake.body) assert(soldier.shield_charge);
    const auto follower_cell = g.spawn_snake.body[1].position;
    const int hp = g.spawn_snake.body[1].hp;
    g.spawn_snake.DamageNode(follower_cell, 1);
    assert(g.spawn_snake.body[1].hp == hp && !g.spawn_snake.body[1].shield_charge);
    g.spawn_snake.body[1].ghost_time = 0;
    g.spawn_snake.DamageNode(follower_cell, 1);
    assert(g.spawn_snake.body[1].hp == hp - 1);
    Soldier detached;
    assert(g.spawn_snake.DetachFollower(2, detached) && detached.shield_charge);
    g.detached_soldiers.push_back(detached);
    g.DamageDetachedSoldier(detached.position, 1);
    assert(g.detached_soldiers[0].hp == detached.hp && !g.detached_soldiers[0].shield_charge);
    g.detached_soldiers.clear();
    auto* near = new SpellTarget({11, 10});
    auto* middle = new SpellTarget({13, 10});
    auto* far = new SpellTarget({15, 10});
    auto* fourth = new SpellTarget({16, 10});
    g.enemies.emplace_back(far); g.enemies.emplace_back(near); g.enemies.emplace_back(middle); g.enemies.emplace_back(fourth);
    equip(ItemEffect::LightningChain); g.UseLeaderItem();
    assert(near->HP() == 2 && middle->HP() == 2 && far->HP() == 2 && fourth->HP() == 3);
    assert(g.world_spells.back().cells[1] == near->GetPosition());
    g.enemies.clear(); g.world_spells.clear();
    equip(ItemEffect::LightningChain); g.UseLeaderItem();
    assert(g.inventory.items[0].cooldown_remaining == 0); // No target doesn't consume cooldown.
    equip(ItemEffect::WaterWall); g.UseLeaderItem();
    assert(g.world_spells.back().cells.size() == 3);
    assert(g.SpellBlocksEnemy({15, 10}) && g.board.IsWalkable({15, 10}));
    assert(g.BlocksProjectile({15, 10}));
    g.state.currentScreen = PAUSE_MENU; g.FixUpdate(10);
    assert(g.world_spells.back().remaining == 5);
    g.state.currentScreen = STAGE; g.UpdateSpellTimers(5);
    assert(!g.SpellBlocksEnemy({15, 10}));
    auto* pillar_target = new SpellTarget({12, 10});
    g.enemies.emplace_back(pillar_target);
    equip(ItemEffect::EarthPillar); g.UseLeaderItem();
    assert(pillar_target->HP() == 2 && g.SpellBlocksEnemy({12, 10}));
    g.world_spells.clear(); g.enemies.clear();
    equip(ItemEffect::IceSpike); g.UseLeaderItem();
    assert(g.projectiles.back().visual == ProjectileVisual::IceSpike && g.projectiles.back().slow_duration == 2);
    auto* ice_target = new SpellTarget({11, 10}); g.enemies.emplace_back(ice_target);
    g.UpdateProjectiles(.05f); assert(ice_target->HP() == 2 && g.projectiles.empty());
    g.enemies.clear();
    equip(ItemEffect::GraveBloom); g.UseLeaderItem();
    assert(g.world_spells.size() == 1 && g.world_spells[0].effect == ItemEffect::GraveBloom);
    const auto bloom = g.world_spells[0].cells[0];
    assert(g.board.GetCellType(bloom) == GROUND);
    auto* bloom_target = new SpellTarget({bloom.x + 1, bloom.y}); g.enemies.emplace_back(bloom_target);
    g.UpdateBloomAttacks(.5f); assert(bloom_target->HP() == 3);
    g.UpdateBloomAttacks(.5f); assert(bloom_target->HP() == 2);
    g.inventory.items.push_back(GetAuraItems()[2]);
    g.spawn_snake.body[0].hp = 1;
    assert(g.LeaderSpellDamage() == 2);
    g.inventory.items.pop_back(); assert(g.LeaderSpellDamage() == 1);
    assert(g.Transition(DoorSide::RIGHT) && g.world_spells.empty());
    assert(g.InitGameObject() && g.world_spells.empty());
    for (const auto& soldier : g.spawn_snake.body) assert(!soldier.shield_charge);
}
