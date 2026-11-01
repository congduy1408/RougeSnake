#include "include/game.h"


game::game(const RoomCatalog& catalog) : room_catalog(catalog) { }

bool game::InitGameObject() {
    
    state = gamestate();
    state.currentScreen = MAIN_MENU;
    combo_counter = 0;
    score_multiplier = 1;
    detached_soldiers.clear();

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
    spawn_food.ResetScore();

    TraceLog(LOG_INFO, "Spawning food...");
    spawn_food.SetFoodPosition(spawn_snake, board);
    last_get_time = GetTime();
    TraceLog(LOG_INFO, "Initialization completed");

    InitRoomObject();

    return true;
}

void game::InitRoomObject() {
    enemies.clear();
    for (const auto& marker : room.GetObjectMarkers()) {
        if (marker.name == "enemy") {
            TraceLog(LOG_ERROR, "load enemy: %s", marker.name);
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
    // spawn snake + food
    TraceLog(LOG_INFO, "Resetting snake..."); 
    spawn_snake.EnterRoom(room);

    TraceLog(LOG_INFO, "Spawning food...");
    spawn_food.SetFoodPosition(spawn_snake, board);

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
            spawn_food.Draw(textures);
            for (const auto& enemy : enemies) {
                enemy->Draw();
            }
            DrawText(TextFormat("Score: %.1f", state.score), 10,10, 20, darkGreen);
            DrawText(TextFormat("Combo: %d", combo_counter), 10,35, 20, darkGreen);
            DrawText(TextFormat("Multiplier: %.1fx", score_multiplier), 10,60, 20, darkGreen);
        } break;
        case GAMEOVER: {
            DrawText(TextFormat("Score: %.1f", state.score), screenWidth/2 ,screenHeight/2, 50, darkGreen);
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
    // update food
    spawn_food.Update();
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

        // check snake collision with food
        //// grow snake, get score, update combo, spawn new food
        //// reset combo if snake move out of food boundary
        if (spawn_food.IsAt(head_pos)) {
            HandleFoodCollision();
        }
        else if (spawn_food.UpdateBoundaryScore(head_pos))
        {
            combo_counter = 0;
            score_multiplier = 1.0f;
        }
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
    // process enemy attack
    for (const auto& action : actions_list) {
        ProcessEnemyAttack(action);
        if (state.currentScreen == GAMEOVER) {
            return;
        }
    }

    // remove dead enemies
    CleanupDeadEnemies();
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

void game::HandleFoodCollision() {
    int food_score = spawn_food.GetScore();
    UpdateComboCounter(food_score, spawn_food.max_score);
    state.score += GetFoodScoreWithCombo(food_score);
    spawn_snake.Grow();
    spawn_food.SetFoodPosition(spawn_snake, board);
}

void game::UpdateComboCounter(int food_score, int food_max_score) {
    if (food_score != food_max_score) {
        combo_counter = 0;
        score_multiplier = 1;
        return;
    }

    combo_counter++;
    if (combo_score_step_count <= 0 || combo_counter % combo_score_step_count != 0) {
        return;
    }

    if (score_multiplier < score_multiplier_start) {
        score_multiplier = score_multiplier_start;
    } else if (score_multiplier < score_multiplier_max) {
        score_multiplier += score_multiplier_step;
        if (score_multiplier > score_multiplier_max) {
            score_multiplier = score_multiplier_max;
        }
    }
}

float game::GetFoodScoreWithCombo(int food_score) {
    return food_score * score_multiplier;
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



