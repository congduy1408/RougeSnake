// Standalone integration harness; not part of the game build.
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

int main() {
    RoomCatalog catalog;
    assert(catalog.LoadAll("map/map_manifest.json"));
    game g(catalog);
    assert(g.InitGameObject());
    g.healing_shards = 20;
    g.spawn_snake.body[0].stats.max_hp = 10;
    g.spawn_snake.body[0].hp = 1;
    assert(g.HealSoldier(0) && g.spawn_snake.body[0].hp == 2 && g.healing_shards == 19);
    g.inventory.items = {GetDawnVeil(), GetDawnVeil(), GetFillerItems()[0]};
    g.inventory.selected = 2;
    assert(g.HealSoldier(0) && g.spawn_snake.body[0].hp == 4 && g.healing_shards == 18);
    g.inventory.UpdateCooldowns(100000);
    assert(g.HealSoldier(0) && g.spawn_snake.body[0].hp == 6);
    g.spawn_snake.body[1].hp = 1;
    assert(g.HealSoldier(1) && g.spawn_snake.body[1].hp == 3);
    const int supply = g.healing_shards;
    assert(!g.HealSoldier(1) && g.healing_shards == supply);
    g.spawn_snake.body[1].hp = 2;
    assert(g.HealSoldier(1) && g.spawn_snake.body[1].hp == 3);
    g.inventory.items = {GetDawnVeil(), GetFillerItems()[0], GetFillerItems()[1],
                        GetFillerItems()[2], GetFillerItems()[4], GetFillerItems()[5]};
    g.inventory.selected = 0;
    g.inventory.cipher = "DDDD"; // Six slots, with no other D item.
    assert(g.inventory.CipherMatches()[0]);
    const GridPosition head = g.spawn_snake.GetHeadPosition();
    g.ground_items.push_back({GetFillerItems()[6], head, false});
    g.CollectCombatItem();
    assert(g.ground_items.empty() && !g.inventory.HasAura(ItemEffect::DawnVeil));
    assert(!g.inventory.CipherMatches()[0]);
    g.spawn_snake.body[0].hp = 1;
    assert(g.HealSoldier(0) && g.spawn_snake.body[0].hp == 2);
    g.ground_items.push_back({GetDawnVeil(), head, false});
    g.CollectCombatItem();
    assert(g.inventory.HasAura(ItemEffect::DawnVeil) && g.ground_items.size() == 1);
    assert(g.ground_items[0].item.id == "moonlit_talisman");
    g.CollectCombatItem(); // Dropped non-aura remains protected until leaving.
    assert(g.inventory.HasAura(ItemEffect::DawnVeil));
    g.inventory.selected = 1;
    assert(g.Transition(DoorSide::RIGHT));
    assert(g.inventory.HasAura(ItemEffect::DawnVeil) && g.ground_items.empty());
    assert(g.InitGameObject());
    assert(!g.inventory.HasAura(ItemEffect::DawnVeil));
    g.SpawnTestItem(ItemCategory::Aura);
    assert(g.ground_items.size() == 1 && g.ground_items[0].item.id == "dawn_veil");
}
