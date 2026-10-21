#include "include/game.h"


game::game(const RoomCatalog& catalog) : room_catalog(catalog) { }

bool game::InitGameObject() {
    
    state = gamestate();
    state.currentScreen = MAIN_MENU;
    combo_counter = 0;
    score_multiplier = 1;

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
    // Update snake movement base on interval
    bool snake_moved = spawn_snake.UpdateMovement(interval);
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

    for(auto& enemy: enemies) {
        //move enemy
        bool enemy_moved = enemy->Update(board, interval);
        if (enemy->IsAt(spawn_snake.GetHeadPosition()))
        {
            // handle attack
            // apply damage to enemies
            if ((snake_moved || enemy_moved) &&
                enemy->IsAt(spawn_snake.GetHeadPosition()))
            {
                enemy->OnSnakeEnter(spawn_snake);
            }
        }
    }

    // remove dead enemies
    CleanupDeadEnemies();
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
