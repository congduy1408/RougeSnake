// Run from Untitled/, linked with production objects except main.o.
#include "include/game.h"
#include <cassert>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <cstdio>

int main() {
    Board board(40, 30);
    for (int x = 0; x < board.GetWidth(); ++x)
        for (int y = 0; y < board.GetHeight(); ++y) board.SetCell({x, y}, GROUND);
    Snake squad;
    squad.body.emplace_back();
    squad.body.front().position = {6, 6};
    std::vector<Soldier> waiting;
    Enemy zombie({5, 5});
    assert(zombie.GetType() == EnemyType::GraveZombie);
    assert(std::string(zombie.GetName()) == "Grave Zombie");
    zombie.OnSnakeEnter(squad);
    assert(squad.body.front().hp == 3); // No contact damage.
    assert(zombie.Update(board, squad, waiting, 0).attack_zone.empty());
    assert(zombie.Update(board, squad, waiting, .25f).attack_zone.empty());
    squad.body.front().position = {30, 20};
    EnemyAction hit = zombie.Update(board, squad, waiting, .25f);
    assert(hit.damage == 1 && hit.attack_zone.size() == 1);
    assert(hit.attack_zone.front() == GridPosition({6, 6})); // Locked target, not retargeted.
    assert(zombie.GetPosition() == GridPosition({5, 5}));
    assert(zombie.Update(board, squad, waiting, .08f).attack_zone.empty());
    assert(zombie.Update(board, squad, waiting, 1).attack_zone.empty());
    zombie.TakeDamage(2); assert(!zombie.IsDead());
    zombie.TakeDamage(1); assert(zombie.IsDead()); // Exactly three HP.
    assert(zombie.Update(board, squad, waiting, 10).attack_zone.empty());
    Enemy waiting_target({5, 5});
    waiting.emplace_back(); waiting.front().position = {6, 5};
    waiting_target.Update(board, squad, waiting, 0);
    assert(waiting_target.Update(board, squad, waiting, .5f).attack_zone.front() == GridPosition({6, 5}));
    waiting.clear();
    for (GridPosition cell : std::vector<GridPosition>{{4, 5}, {6, 5}, {5, 4}, {5, 6}})
        board.SetCell(cell, GAP);
    Enemy trapped({5, 5});
    for (int i = 0; i < 20; ++i) trapped.Update(board, squad, waiting, .1f);
    assert(trapped.GetPosition() == GridPosition({5, 5})); // Never enters lethal terrain.

    RoomCatalog catalog;
    assert(catalog.LoadAll("map/map_manifest.json"));
    game g(catalog);
    assert(g.InitGameObject());
    const auto original_count = g.enemies.size();
    assert(original_count > 0);
    for (const auto& enemy : g.enemies) assert(enemy->GetType() == EnemyType::GraveZombie);
    std::ifstream original("map/testmap_down.json");
    assert(original);
    std::string json((std::istreambuf_iterator<char>(original)), std::istreambuf_iterator<char>());
    const std::string legacy = "\"enemy\"", named = "\"grave_zombie\"";
    std::size_t offset = 0;
    while ((offset = json.find(legacy, offset)) != std::string::npos) {
        json.replace(offset, legacy.size(), named); offset += named.size();
    }
    char path[] = "/tmp/rougesnake-zombie-map-XXXXXX";
    int fd = mkstemp(path); assert(fd >= 0); close(fd);
    { std::ofstream out(path); out << json; assert(out.good()); }
    RoomTemplate room;
    assert(room.LoadFromFile(path));
    std::remove(path);
    assert(g.room.Load(room, g.board));
    g.InitRoomObject();
    assert(g.enemies.size() == original_count);
    for (const auto& enemy : g.enemies) assert(std::string(enemy->GetName()) == "Grave Zombie");
}
