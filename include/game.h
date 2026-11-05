#pragma once
#include "include/common.h"
#include "include/snake.h"
#include "include/gamestate.h"
#include "include/board.h"
#include "include/room_template.h"
#include "include/room.h"
#include "include/game_textures.h"
#include "include/enemy.h"
#include "include/soldier.h"

// Natural recruitment tuning; terrain and skill bonuses feed the chance calculation.
struct RecruitSpawnSettings {
    float interval = 5.0f;
    float base_chance = 0.5f;
    float skill_multiplier = 1.0f;
    std::size_t max_waiting = 2;
    std::vector<std::pair<SoldierType, int>> type_weights = {
        {SoldierType::Basic, 60}, {SoldierType::Veteran, 20}, {SoldierType::Archer, 20}
    };
};

class game {
    private:
        const RoomCatalog& room_catalog;
        DoorSide GetOppositeSide(DoorSide side);
        void ProcessEnemyAttack(const EnemyAction& action);
        void ProcessSoldierAttack(const SoldierAction& action);
        void UpdateProjectiles(float delta_time);
        bool BlocksProjectile(GridPosition cell) const;
        bool DamageEnemyAt(GridPosition cell, int damage);
        void DrawSoldierAttacks() const;
        void DamageDetachedSoldier(GridPosition position, int damage);
        void ReduceDetachedGhostTime(float delta_time);
        void DrawDetachedSoldiers(const GameTextures& textures) const;
        void HandleSoldierRescue();
        void UpdateRecruitSpawning(float delta_time);
        bool FindRecruitSpawnCell(GridPosition& cell) const;
        float GetRecruitSpawnChance(GridPosition cell) const;
        float recruit_spawn_elapsed = 0.0f;
    public:
        gamestate state;

        Board board{cellcount_width, cellcount_height};
        
        Snake spawn_snake;
        RoomTemplate room_template;
        Room room;
        std::vector<std::unique_ptr<Enemy>> enemies;
        std::vector<Soldier> detached_soldiers;
        std::vector<Projectile> projectiles;

        RecruitSpawnSettings recruit_spawn;

        game(const RoomCatalog& catalog);
        bool InitGameObject();
        void InitRoomObject();
        bool Transition(DoorSide side);
        void Draw(const GameTextures& textures);
        void ReadInput();
        void FixUpdate(float interval);

        // bool SnakeCollision(const Snake& snake, const GameObject& object) const;
        
        void CleanupDeadEnemies();
};
