// Standalone production-code harness, excluded from the game Makefile.
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

class Target : public Enemy {
public:
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
    g.spawn_snake.body[0].position = {5, 5};
    g.inventory.items = {GetFireball(), GetFireball()};
    assert(GetFireball().category == ItemCategory::Spell && GetFireball().letter == 'F');
    Snake movement_probe;
    movement_probe.Reset(g.room);
    assert(!movement_probe.UpdateMovement(0.05f)); // Fireball advances a cell in this time.
    const direction directions[] = {dir_up, dir_down, dir_left, dir_right};
    const GridPosition aims[] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    for (int i = 0; i < 4; ++i) {
        g.projectiles.clear();
        g.inventory.items[0].cooldown_remaining = 0;
        g.spawn_snake.snake_move = directions[i];
        g.UseLeaderItem();
        assert(g.projectiles.size() == 1 && g.projectiles[0].direction == aims[i]);
        assert(g.projectiles[0].visual == ProjectileVisual::Fireball);
        assert(g.projectiles[0].speed == 20.0f);
        g.UseLeaderItem();
        assert(g.projectiles.size() == 1 && g.inventory.items[0].cooldown_remaining == 2);
        g.UpdateProjectiles(0.05f);
        assert(g.projectiles[0].position == GridPosition({5 + aims[i].x, 5 + aims[i].y}));
    }
    g.projectiles.clear();
    g.inventory.selected = 1; // Other copy has an independent cooldown.
    g.spawn_snake.snake_move = dir_right;
    g.UseLeaderItem();
    assert(g.projectiles.size() == 1);
    g.state.currentScreen = PAUSE_MENU;
    g.FixUpdate(10);
    assert(g.inventory.items[1].cooldown_remaining == 2);
    assert(g.projectiles[0].position == GridPosition({5, 5}));
    g.UseLeaderItem();
    assert(g.projectiles.size() == 1);
    g.state.currentScreen = STAGE;
    auto* first = new Target({6, 5});
    auto* second = new Target({7, 5});
    g.enemies.emplace_back(first);
    g.enemies.emplace_back(second);
    g.UpdateProjectiles(0.5f); // Crossed cells cannot skip the nearest target.
    assert(first->HP() == 2 && second->HP() == 3 && g.projectiles.empty());
    g.enemies.clear();
    auto cast = [&]() {
        g.inventory.items[1].cooldown_remaining = 0;
        g.UseLeaderItem();
    };
    g.board.SetCell({6, 5}, WALL);
    cast(); g.UpdateProjectiles(0.2f);
    assert(g.projectiles.empty());
    g.board.SetCell({6, 5}, GAP);
    cast(); g.UpdateProjectiles(0.05f);
    assert(g.projectiles.size() == 1 && g.projectiles[0].position == GridPosition({6, 5}));
    g.UpdateProjectiles(1);
    assert(g.projectiles.empty()); // Eight-cell range expires.
    cast();
    assert(g.Transition(DoorSide::RIGHT));
    assert(g.projectiles.empty() && g.inventory.items.size() == 2);
    assert(g.inventory.items[1].cooldown_remaining == 2);
    assert(g.InitGameObject());
    assert(g.projectiles.empty() && g.inventory.items.empty());
    g.SpawnTestItem(ItemCategory::Spell);
    assert(g.ground_items.size() == 1 && g.ground_items[0].item.id == "fireball");
    Projectile arrow({{1, 1}, {1, 0}, 10, 8, 1});
    assert(arrow.visual == ProjectileVisual::Arrow);
}
