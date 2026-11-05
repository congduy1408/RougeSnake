#include "include/game.h"


game::game(const RoomCatalog& catalog) : room_catalog(catalog) { }

bool game::InitGameObject() {
    
    state = gamestate();
    state.currentScreen = MAIN_MENU;
    recruit_spawn_elapsed = 0.0f;
    detached_soldiers.clear();
    projectiles.clear();

    if (!room_template.LoadFromFile("map/testmap_down.json")) {
        TraceLog(LOG_ERROR, "Failed to load room template");
        return false;
    }
    TraceLog(LOG_INFO, "Loading room...");
    if (!room.Load(room_template, board)) {
        TraceLog(LOG_ERROR, "Failed to load room");
        return false;        
    }
    TraceLog(LOG_INFO, "Resetting snake...");
    spawn_snake.Reset(room);
    TraceLog(LOG_INFO, "Initialization completed");

    InitRoomObject();

    return true;
}

void game::InitRoomObject() {
    enemies.clear();
    for (const auto& marker : room.GetObjectMarkers()) {
        if (marker.name == "enemy") {
            TraceLog(LOG_INFO, "load enemy: %s", marker.name.c_str());
            // Create enemy at marker.position
            enemies.push_back(std::make_unique<Enemy>(marker.position));
        }
        else if (marker.name == "item") {
            // Create item at marker.position
        }
    }
}

bool game::Transition(DoorSide side) {
    // read random map from door side
    DoorSide opposite_side = GetOppositeSide(side);
    // load room template
    std::string room_path = room_catalog.SelectRandom(opposite_side);
    if (room_path.empty()) {
        TraceLog(LOG_ERROR, "Empty room list");
        return false;
    }
    RoomTemplate temp_template;
    if (!temp_template.LoadFromFile(room_path)) {
        TraceLog(LOG_ERROR, "Failed to load room template");
        return false;
    }
    // load room + board
    if (!room.Load(temp_template, board)) {
        TraceLog(LOG_ERROR, "Failed to load room");
        return false;        
    }
    room_template = std::move(temp_template);
    projectiles.clear();
    // Unrescued soldiers are left behind when the squad changes rooms.
    detached_soldiers.clear();
    // Move the attached squad into the new room.
    TraceLog(LOG_INFO, "Resetting snake..."); 
    spawn_snake.EnterRoom(room);

    InitRoomObject();

    return true;
}

DoorSide game::GetOppositeSide(DoorSide side) {
    switch (side) {
        case DoorSide::DOWN:
            return DoorSide::UP;
            break;
        case DoorSide::UP:
            return DoorSide::DOWN;
            break;
        case DoorSide::LEFT:
            return DoorSide::RIGHT;
            break;
        case DoorSide::RIGHT:
            return DoorSide::LEFT;
            break;
        default:
            return DoorSide::INVALID;
            break;
    }
}

void game::Draw(const GameTextures& textures) {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            DrawText("Press Enter to start game", screenWidth/2 ,screenHeight/2, 20, darkGreen);
        } break;
        case STAGE: {
            board.Draw(textures);
            DrawDetachedSoldiers(textures);
            spawn_snake.Draw(textures);
            for (const auto& enemy : enemies) {
                enemy->Draw();
            }
            DrawSoldierAttacks();
            for (const auto& projectile : projectiles) projectile.Draw();
            DrawText(TextFormat("Followers: %d/%d",
                static_cast<int>(spawn_snake.GetFollowerCount()),
                static_cast<int>(spawn_snake.max_followers)), 10, 10, 20, darkGreen);
        } break;
        case GAMEOVER: {
            DrawText("Leader defeated", screenWidth/2, screenHeight/2, 30, darkGreen);
            DrawText("Press Enter to restart game", screenWidth/2 + 20,screenHeight/2 + 20, 20, darkGreen);

        } break;
        default:
            break;
    }
}

void game::DrawDetachedSoldiers(const GameTextures& textures) const {
    Rectangle sprite = textures.GetSpriteRect(SpriteID::SNAKE_BODY);

    for (const auto& soldier : detached_soldiers) {
        if (soldier.ghost_time > 0.0f) {
            float elapsed = Soldier::initial_ghost_time - soldier.ghost_time;
            if (static_cast<int>(elapsed / 0.05f) % 2 != 0) continue;
        }
        Rectangle destination = {
            soldier.position.x * cellsize + cellsize / 2.0f,
            soldier.position.y * cellsize + cellsize / 2.0f,
            static_cast<float>(cellsize),
            static_cast<float>(cellsize)
        };

        float rotation = 0.0f;
        switch (soldier.cur_dir) {
            case dir_right: rotation = 0.0f; break;
            case dir_down:  rotation = 90.0f; break;
            case dir_left:  rotation = 180.0f; break;
            case dir_up:    rotation = -90.0f; break;
        }

        const Color tint = soldier.hp >= 3 ? WHITE : (soldier.hp == 2 ? BLUE : RED);
        DrawTexturePro(
            textures.GetTexture(),
            sprite,
            destination,
            {cellsize / 2.0f, cellsize / 2.0f},
            rotation,
            tint
        );
    }
}

void game::ReadInput() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            if (IsKeyPressed(KEY_ENTER))
            {
                state.currentScreen = STAGE;
            }
        } break;
        case STAGE: {
            spawn_snake.ReadInput();
        } break;
        case GAMEOVER: {
            if (IsKeyPressed(KEY_ENTER))
            {
                InitGameObject();
                state.currentScreen = STAGE;
            }
        } break;
        default: break;
    }

}

void game::FixUpdate(float interval) {
    if (state.currentScreen != STAGE) {
        return;
    }
    if (spawn_snake.IsDead()) {
        state.currentScreen = GAMEOVER;
        return;
    }
    // Update snake movement base on interval
    bool snake_moved = spawn_snake.UpdateMovement(interval);
    spawn_snake.ReduceGhostTime(interval);
    ReduceDetachedGhostTime(interval);
    spawn_snake.FixedUpdateAnimation(interval);
    // check snake collision with wall
    if (snake_moved)
    {
        GridPosition head_pos = spawn_snake.GetHeadPosition();
        //// move to game over
        if (!board.IsWalkable(head_pos)) {
            state.currentScreen = GAMEOVER;
            return;
        }
        if (board.IsLethal(head_pos)) {
            state.currentScreen = GAMEOVER;
            return;
        }
        // Check snake move to exit door
        if (board.IsDoor(head_pos)) {
            // check door side
            DoorSide exit_side = room.GetDoorSide(head_pos);
            // call change room function
            if (exit_side != DoorSide::INVALID) {
                TraceLog(LOG_INFO, "Snake reached exit door %d", exit_side);
                
                // adding transition logic
                Transition(exit_side);
                return; // stop process other gameobject for prepare next room
            }
        }
        HandleSoldierRescue();

    }
    // process enemy
    // collect enemy attack
    std::vector<EnemyAction> actions_list;
    for(auto& enemy: enemies) {
        if (enemy->IsDead()) continue;
        EnemyAction action = enemy->Update(board, spawn_snake, detached_soldiers, interval);
        if (!action.attack_zone.empty()) {
            actions_list.push_back(action);
        }
    }
    // Observe after enemy movement; collect both sides before applying damage.
    std::vector<SoldierAction> soldier_actions;
    for (std::size_t i = 1; i < spawn_snake.body.size(); ++i) {
        SoldierAction action = spawn_snake.body[i].UpdateCombat(enemies, true, interval);
        if (!action.attack_zone.empty() || !action.projectiles.empty()) soldier_actions.push_back(action);
    }
    for (auto& soldier : detached_soldiers) {
        SoldierAction action = soldier.UpdateCombat(enemies, false, interval);
        if (!action.attack_zone.empty() || !action.projectiles.empty()) soldier_actions.push_back(action);
    }
    // process enemy attack
    for (const auto& action : actions_list) {
        ProcessEnemyAttack(action);
        if (state.currentScreen == GAMEOVER) {
            return;
        }
    }

    // Existing arrows move first; new arrows begin traveling next fixed update.
    UpdateProjectiles(interval);
    // Released strikes survive an attacker's death, unless the leader's death ended the run.
    for (const auto& action : soldier_actions) ProcessSoldierAttack(action);

    // remove dead enemies
    CleanupDeadEnemies();
    UpdateRecruitSpawning(interval);
}

void game::ProcessSoldierAttack(const SoldierAction& action) {
    if (state.currentScreen != STAGE) return;
    if (action.damage > 0) {
        for (const auto& cell : action.attack_zone) DamageEnemyAt(cell, action.damage);
    }
    for (const auto& spawn : action.projectiles) {
        if (spawn.damage <= 0 || spawn.speed <= 0.0f || spawn.max_distance <= 0 ||
            std::abs(spawn.direction.x) > 1 || std::abs(spawn.direction.y) > 1 ||
            spawn.direction == GridPosition{} || BlocksProjectile(spawn.position)) continue;
        projectiles.emplace_back(spawn);
    }
}

// Arrow terrain differs from walking: gaps can be crossed, empty/outside cells cannot.
bool game::BlocksProjectile(GridPosition cell) const {
    if (!board.IsInside(cell)) return true;
    CellType type = board.GetCellType(cell);
    return type == WALL || type == NONE;
}

// Resolve one enemy hit without erasing during attack processing.
bool game::DamageEnemyAt(GridPosition cell, int damage) {
    for (auto& enemy : enemies) {
        if (!enemy->IsDead() && enemy->GetPosition() == cell) {
            enemy->TakeDamage(damage);
            return true;
        }
    }
    return false;
}

// Check every crossed cell so fast arrows cannot skip obstacles or victims.
void game::UpdateProjectiles(float delta_time) {
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        bool finished = it->remaining_steps <= 0 || BlocksProjectile(it->position);
        if (!finished && it->has_moved) finished = DamageEnemyAt(it->position, it->damage);
        it->movement_progress += it->speed * delta_time;
        while (!finished && it->movement_progress >= 1.0f) {
            GridPosition next = {it->position.x + it->direction.x,
                                 it->position.y + it->direction.y};
            bool blocked_corner = it->direction.x != 0 && it->direction.y != 0 &&
                (BlocksProjectile({next.x, it->position.y}) ||
                 BlocksProjectile({it->position.x, next.y}));
            if (blocked_corner || BlocksProjectile(next)) {
                finished = true;
                break;
            }
            it->position = next;
            it->has_moved = true;
            it->movement_progress -= 1.0f;
            --it->remaining_steps;
            finished = DamageEnemyAt(next, it->damage) || it->remaining_steps <= 0;
        }
        if (finished) it = projectiles.erase(it);
        else ++it;
    }
}

void game::DrawSoldierAttacks() const {
    for (std::size_t i = 1; i < spawn_snake.body.size(); ++i) {
        spawn_snake.body[i].DrawAttack();
    }
    for (const auto& soldier : detached_soldiers) {
        if (soldier.CanAttack(false)) soldier.DrawAttack();
    }
}

void game::ProcessEnemyAttack(const EnemyAction& action) {
    if (state.currentScreen != STAGE) return;

    for (const auto& cell : action.attack_zone) {
        const bool attached_at_cell = std::any_of(
            spawn_snake.body.begin(), spawn_snake.body.end(),
            [&](const Soldier& soldier) { return soldier.position == cell; });
        if (attached_at_cell) {
            std::vector<Soldier> detached = spawn_snake.DamageNode(cell, action.damage);
            detached_soldiers.insert(detached_soldiers.end(), detached.begin(), detached.end());
        } else {
            DamageDetachedSoldier(cell, action.damage);
        }

        if (spawn_snake.IsDead()) {
            state.currentScreen = GAMEOVER;
            return;
        }
    }
}

void game::DamageDetachedSoldier(GridPosition position, int damage) {
    if (damage <= 0) return;

    for (auto it = detached_soldiers.begin(); it != detached_soldiers.end(); ++it) {
        if (it->hp <= 0 || !(it->position == position)) continue;
        if (it->ghost_time > 0.0f) return;

        it->hp -= damage;
        if (it->hp <= 0) {
            detached_soldiers.erase(it);
        } else {
            it->ghost_time = Soldier::initial_ghost_time;
        }
        return; // One victim per cell, including when that victim dies.
    }
}

void game::ReduceDetachedGhostTime(float delta_time) {
    for (auto& soldier : detached_soldiers) {
        soldier.ghost_time = std::max(0.0f, soldier.ghost_time - delta_time);
    }
}

void game::HandleSoldierRescue() {
    const GridPosition head_pos = spawn_snake.GetHeadPosition();
    for (auto it = detached_soldiers.begin(); it != detached_soldiers.end(); ++it) {
        if (it->hp > 0 && it->position == head_pos) {
            if (spawn_snake.AttachSoldier(*it)) {
                detached_soldiers.erase(it);
            }
            // Rescue at most one soldier per leader movement step.
            return;
        }
    }
}

// Reservoir sampling selects uniformly among valid cells without random retry loops.
bool game::FindRecruitSpawnCell(GridPosition& cell) const {
    int candidates = 0;
    for (int x = 0; x < board.GetWidth(); ++x) {
        for (int y = 0; y < board.GetHeight(); ++y) {
            GridPosition candidate = {x, y};
            if (board.GetCellType(candidate) != GROUND) continue;
            bool occupied = std::any_of(spawn_snake.body.begin(), spawn_snake.body.end(),
                [&](const Soldier& soldier) { return soldier.position == candidate; });
            occupied = occupied || std::any_of(detached_soldiers.begin(), detached_soldiers.end(),
                [&](const Soldier& soldier) { return soldier.hp > 0 && soldier.position == candidate; });
            occupied = occupied || std::any_of(enemies.begin(), enemies.end(),
                [&](const std::unique_ptr<Enemy>& enemy) {
                    return !enemy->IsDead() && enemy->GetPosition() == candidate;
                });
            if (!occupied && GetRandomValue(1, ++candidates) == 1) cell = candidate;
        }
    }
    return candidates > 0;
}

// Extend this terrain multiplier when recruit-friendly terrain is introduced.
float game::GetRecruitSpawnChance(GridPosition cell) const {
    float terrain_multiplier = board.GetCellType(cell) == GROUND ? 1.0f : 0.0f;
    return std::max(0.0f, std::min(1.0f,
        recruit_spawn.base_chance * terrain_multiplier * recruit_spawn.skill_multiplier));
}

// One scheduled attempt, independent of pickup/death; blocked attempts never build a backlog.
void game::UpdateRecruitSpawning(float delta_time) {
    if (recruit_spawn.interval <= 0.0f) {
        recruit_spawn_elapsed = 0.0f;
        return;
    }
    recruit_spawn_elapsed += std::max(0.0f, delta_time);
    if (recruit_spawn_elapsed < recruit_spawn.interval) return;
    recruit_spawn_elapsed = 0.0f;

    if (!spawn_snake.HasFollowerCapacity()) return;
    const auto waiting = std::count_if(detached_soldiers.begin(), detached_soldiers.end(),
        [](const Soldier& soldier) { return soldier.hp > 0; });
    if (static_cast<std::size_t>(waiting) >= recruit_spawn.max_waiting) return;

    GridPosition cell;
    if (!FindRecruitSpawnCell(cell)) return;
    if (GetRandomValue(0, 9999) >= GetRecruitSpawnChance(cell) * 10000.0f) return;

    int total_weight = 0;
    for (const auto& entry : recruit_spawn.type_weights) total_weight += std::max(0, entry.second);
    if (total_weight <= 0) return;
    int roll = GetRandomValue(1, total_weight);
    for (const auto& entry : recruit_spawn.type_weights) {
        roll -= std::max(0, entry.second);
        if (roll <= 0) {
            Soldier recruit(entry.first);
            recruit.position = cell;
            detached_soldiers.push_back(recruit);
            return;
        }
    }
}

void game::CleanupDeadEnemies() {
    enemies.erase(
        std::remove_if(
            enemies.begin(),
            enemies.end(),
            [](const std::unique_ptr<Enemy>& enemy) {
                return enemy->IsDead();
            }
        ),
        enemies.end()
    );
}



