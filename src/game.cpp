#include "include/game.h"

#include <algorithm>
#include <cstdio>

game::game() {
    game_sprite = LoadTexture("sprite/game_sprite.png");
    SetTextureFilter(game_sprite, TEXTURE_FILTER_POINT);
    spawn_food.SetSpriteTexture(game_sprite);
    const char* application_directory = GetApplicationDirectory();
    high_score_path = application_directory != nullptr ? application_directory : "";
    if (!high_score_path.empty() && high_score_path.back() != '/' &&
        high_score_path.back() != '\\') {
        high_score_path += '/';
    }
    high_score_path += "highscore.txt";
    LoadHighScore();
}

game::~game() {
    wall_bricks.clear();
    if (game_sprite.id != 0) {
        UnloadTexture(game_sprite);
    }
}

void game::InitGameObject() {
    state = gamestate();
    state.currentScreen = MAIN_MENU;
    combo_counter = 0;
    score_multiplier = 1;
    score_popup_active = false;
    score_popup_remaining = 0.0f;
    spawn_snake.Reset();
    spawn_snake.SetTint(WHITE);
    enemy_snake.Kill();
    stage_progress.Reset();
    falling_rocks.clear();
    next_rock_spawn_time = 0.0;
    InitGround();
    InitDoors();
    wall_bricks.clear();
    wall_cells.assign(cellcount_width * cellcount_height, false);
    InitStationaryWall();
    spawn_food.Reset(FoodType::Apple);
    RespawnFood();
    last_get_time = GetTime();
}

void game::Draw() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            DrawCenteredText("ROUGE SNAKE", screenHeight / 2 - 90, 40, darkGreen);
            DrawCenteredText(TextFormat("High Score: %d", high_score),
                             screenHeight / 2 - 20, 24, Color{236, 197, 74, 255});
            DrawCenteredText("Press Enter to start game", screenHeight / 2 + 35,
                             20, darkGreen);
        } break;
        case STAGE: {
            DrawGround();
            for (unsigned int i=0; i<wall_bricks.size(); i++) {
                wall_bricks[i].Draw();
            }
            DrawDoors();
            spawn_snake.Draw();
            enemy_snake.Draw();
            spawn_food.Draw();
            for (const FallingRock& rock : falling_rocks) {
                rock.Draw();
            }
            DrawScorePopup();
            DrawStageBanner();
            DrawUI();
        } break;
        case GAMEOVER: {
            DrawCenteredText("GAME OVER", screenHeight / 2 - 100, 42,
                             Color{218, 74, 66, 255});
            DrawCenteredText(TextFormat("Score: %d", state.score),
                             screenHeight / 2 - 35, 28, darkGreen);
            DrawCenteredText(TextFormat("High Score: %d", high_score),
                             screenHeight / 2 + 5, 24, Color{236, 197, 74, 255});
            DrawCenteredText("Press Enter to restart game", screenHeight / 2 + 65,
                             20, darkGreen);
        } break;
        default:
            break;
    }
}
void game::Update() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            if (IsKeyPressed(KEY_ENTER)  || IsKeyPressed(KEY_KP_ENTER))
            {
                state.currentScreen = STAGE;
            }
        } break;
        case STAGE: {
            stage_progress.Update(GetFrameTime());
            UpdateScorePopup();
            UpdateRocks();
            if (state.currentScreen == GAMEOVER) {
                break;
            }
            if (!spawn_snake.IsAlive()) {
                state.currentScreen = GAMEOVER;
                break;
            }
            spawn_snake.SetSpeedBoost(IsKeyDown(KEY_X));
            spawn_snake.ReadInput();
            bool player_moved = FixUpdate(spawn_snake.move_interval, last_get_time);

            if (player_moved) {
                int removed_body_count = spawn_snake.Update();
                ApplyPlayerCutPenalty(removed_body_count, spawn_snake.body.front().position);
                if (!spawn_snake.IsAlive()) {
                    state.currentScreen = GAMEOVER;
                    break;
                }
                if (IsWallCell(spawn_snake.body.front().position)) {
                    state.currentScreen = GAMEOVER;
                    break;
                }
            }

            std::vector<bool> blocked_cells = BuildBlockedCells();
            bool enemy_moved = enemy_snake.Update(blocked_cells);

            if (player_moved || enemy_moved) {
                HandleCrossSnakeCollisions();
                if (state.currentScreen == GAMEOVER) {
                    break;
                }
            }

            if (player_moved) {
                spawn_food.Update();
                if (SnakeCollision(spawn_snake, spawn_food)) {
                    HandlePlayerFoodCollision();
                } else {
                    if (spawn_food.GetFoodType() == FoodType::Apple &&
                        spawn_food.UpdateBoundaryScore(spawn_snake.body.front().position)) {
                        combo_counter = 0;
                        score_multiplier = 1;
                    }
                }
            }

            if (enemy_moved && enemy_snake.IsAlive() &&
                spawn_food.GetFoodType() == FoodType::Apple &&
                SnakeCollision(enemy_snake.GetSnake(), spawn_food)) {
                enemy_snake.GetSnake().Grow();
                RespawnFood();
            }
        } break;
        case GAMEOVER: {
            UpdateHighScore();
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
            {
                
                InitGameObject();
                state.currentScreen = STAGE;
            }
        } break;
        default: break;
    }

}

void game::DrawCenteredText(const char* text, int y, int font_size, Color color) {
    int text_width = MeasureText(text, font_size);
    DrawText(text, (screenWidth - text_width) / 2, y, font_size, color);
}

void game::LoadHighScore() {
    FILE* high_score_file = std::fopen(high_score_path.c_str(), "r");
    int loaded_score = 0;
    if (high_score_file != nullptr &&
        std::fscanf(high_score_file, "%d", &loaded_score) == 1 &&
        loaded_score >= 0) {
        high_score = loaded_score;
    } else {
        high_score = 0;
    }
    if (high_score_file != nullptr) {
        std::fclose(high_score_file);
    }
}

void game::UpdateHighScore() {
    if (state.score <= high_score) {
        return;
    }

    high_score = state.score;
    SaveHighScore();
}

void game::SaveHighScore() {
    FILE* high_score_file = std::fopen(high_score_path.c_str(), "w");
    if (high_score_file != nullptr) {
        std::fprintf(high_score_file, "%d", high_score);
        std::fclose(high_score_file);
    }
}

bool game::FixUpdate(float interval, double& last_update_time) {
    double current_get_time = GetTime();
    if (current_get_time - last_update_time >= interval) {
        last_update_time = current_get_time;
        return true;
    } else {
        return false;
    }
}

int game::FindBodyCollisionIndex(const Snake& moving_snake, const Snake& target_snake) {
    if (moving_snake.body.empty()) {
        return -1;
    }

    for (size_t i = 1; i < target_snake.body.size(); i++) {
        if (Vector2Equals(moving_snake.body.front().position, target_snake.body[i].position)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void game::HandleCrossSnakeCollisions() {
    if (!enemy_snake.IsAlive() || spawn_snake.body.empty()) {
        return;
    }

    Snake& enemy = enemy_snake.GetSnake();
    int enemy_cut_index = FindBodyCollisionIndex(spawn_snake, enemy);
    int player_cut_index = FindBodyCollisionIndex(enemy, spawn_snake);

    if (enemy_cut_index > 0) {
        enemy.TailCut(enemy_cut_index);
    }
    if (player_cut_index > 0) {
        int removed_body_count = spawn_snake.TailCut(player_cut_index);
        ApplyPlayerCutPenalty(removed_body_count, enemy.body.front().position);
    }

    if (!enemy.IsAlive()) {
        enemy_snake.Kill();
    }
    if (!spawn_snake.IsAlive()) {
        state.currentScreen = GAMEOVER;
    }
}

void game::ApplyPlayerCutPenalty(int removed_body_count, Vector2 popup_position) {
    if (removed_body_count <= 0) {
        return;
    }

    int requested_penalty = removed_body_count * 2;
    int deducted_score = requested_penalty < state.score ? requested_penalty : state.score;
    state.score -= deducted_score;
    if (deducted_score > 0) {
        ShowScorePopup(popup_position, -deducted_score);
    }
}

void game::HandlePlayerFoodCollision() {
    Vector2 eaten_position = spawn_food.GetPosition();
    if (spawn_food.GetFoodType() == FoodType::Key) {
        AdvanceStage();
        return;
    }

    int food_score = spawn_food.GetScore();
    UpdateComboCounter(food_score, spawn_food.max_score);
    int gained_score = GetFoodScoreWithCombo(food_score);
    state.score += gained_score;
    ShowScorePopup(eaten_position, gained_score);
    spawn_snake.Grow();

    if (stage_progress.RecordAppleEaten()) {
        spawn_food.Reset(FoodType::Key);
    } else {
        spawn_food.Reset(FoodType::Apple);
    }
    RespawnFood();
}

void game::AdvanceStage() {
    stage_progress.Advance();
    spawn_food.Reset(FoodType::Apple);
    combo_counter = 0;
    score_multiplier = 1.0f;

    if (stage_progress.GetStageIndex() == 1) {
        ScheduleNextRock();
    }
    if (stage_progress.GetStageIndex() == 2) {
        enemy_snake.Reset(Vector2{30, 8}, dir_left);
    }
    RespawnFood();
}

void game::RespawnFood() {
    std::vector<bool> blocked_cells = BuildBlockedCells();
    spawn_food.SetFoodPosition(spawn_snake, enemy_snake.GetSnake(), blocked_cells);
}

bool game::SnakeCollision(Snake& snake, GameObject object) {
    if (snake.body.empty()) {
        return false;
    }
    if (snake.body.front().position == object.GetPosition()) {
        return true;
    } else {
        return false;
    }
}

int game::CellIndex(Vector2 pos) {
    return (int)pos.y * cellcount_width + (int)pos.x;
}

bool game::IsWallCell(Vector2 pos) {
    int x = (int)pos.x;
    int y = (int)pos.y;
    if (x < 0 || x >= cellcount_width || y < 0 || y >= cellcount_height) {
        return true;
    }
    int cell_index = CellIndex(pos);
    return cell_index < 0 ||
           static_cast<size_t>(cell_index) >= wall_cells.size() ||
           wall_cells[cell_index] ||
           IsRockCell(pos, true);
}

void game::AddWallBrick(Vector2 pos) {
    int x = (int)pos.x;
    int y = (int)pos.y;
    if (x < 0 || x >= cellcount_width || y < 0 || y >= cellcount_height) {
        return;
    }

    int cell_index = CellIndex(pos);
    if (cell_index < 0 || static_cast<size_t>(cell_index) >= wall_cells.size()) {
        return;
    }
    if (wall_cells[cell_index]) {
        return;
    }

    wall_cells[cell_index] = true;
    wall_bricks.emplace_back(pos, game_sprite);
}

void game::UpdateRocks() {
    float delta_time = GetFrameTime();
    for (FallingRock& rock : falling_rocks) {
        if (rock.Update(delta_time)) {
            HandleRockImpact(rock.GetPosition());
        }
    }

    falling_rocks.erase(
        std::remove_if(falling_rocks.begin(), falling_rocks.end(),
            [](const FallingRock& rock) { return rock.IsExpired(); }),
        falling_rocks.end());

    // Rocks are introduced in Stage 1 and remain active in later stages.
    if (stage_progress.GetStageIndex() < 1) {
        return;
    }
    if (next_rock_spawn_time <= 0.0) {
        ScheduleNextRock();
    }
    if (GetTime() < next_rock_spawn_time) {
        return;
    }

    int falling_count = 0;
    for (const FallingRock& rock : falling_rocks) {
        if (rock.GetState() == RockState::Warning || rock.GetState() == RockState::Falling) {
            falling_count++;
        }
    }
    int falling_capacity = max_falling_rocks - falling_count;
    int map_capacity = max_rocks_on_map - static_cast<int>(falling_rocks.size());
    int wave_capacity = falling_capacity < map_capacity ? falling_capacity : map_capacity;
    if (wave_capacity <= 0) {
        return;
    }

    int wave_size = GetRandomValue(1, wave_capacity);
    for (int i = 0; i < wave_size; i++) {
        if (!SpawnRock()) {
            break;
        }
    }
    ScheduleNextRock();
}

bool game::SpawnRock() {
    constexpr int max_spawn_attempts = 100;
    for (int attempt = 0; attempt < max_spawn_attempts; attempt++) {
        Vector2 position = {
            static_cast<float>(GetRandomValue(1, cellcount_width - 2)),
            static_cast<float>(GetRandomValue(1, cellcount_height - 2))
        };
        int cell_index = static_cast<int>(position.y) * cellcount_width +
                         static_cast<int>(position.x);
        if (wall_cells[cell_index] || IsRockCell(position, false)) {
            continue;
        }

        falling_rocks.emplace_back(position, game_sprite, rock_warning_duration,
                                   rock_fall_duration, rock_lifetime);
        return true;
    }
    return false;
}

void game::ScheduleNextRock() {
    int minimum_centiseconds = static_cast<int>(rock_spawn_interval_min * 100.0f);
    int maximum_centiseconds = static_cast<int>(rock_spawn_interval_max * 100.0f);
    if (maximum_centiseconds < minimum_centiseconds) {
        std::swap(minimum_centiseconds, maximum_centiseconds);
    }
    int delay_centiseconds = GetRandomValue(minimum_centiseconds, maximum_centiseconds);
    next_rock_spawn_time = GetTime() + delay_centiseconds / 100.0;
}

void game::HandleRockImpact(Vector2 position) {
    if (Vector2Equals(spawn_food.GetPosition(), position)) {
        RespawnFood();
    }

    for (size_t i = 0; i < spawn_snake.body.size(); i++) {
        if (!Vector2Equals(spawn_snake.body[i].position, position)) {
            continue;
        }
        if (i == 0) {
            state.currentScreen = GAMEOVER;
        } else {
            int removed_count = spawn_snake.TailCut(static_cast<int>(i));
            ApplyPlayerCutPenalty(removed_count, position);
            if (!spawn_snake.IsAlive()) {
                state.currentScreen = GAMEOVER;
            }
        }
        break;
    }

    if (!enemy_snake.IsAlive()) {
        return;
    }
    Snake& enemy = enemy_snake.GetSnake();
    for (size_t i = 0; i < enemy.body.size(); i++) {
        if (!Vector2Equals(enemy.body[i].position, position)) {
            continue;
        }
        if (i == 0) {
            enemy_snake.Kill();
        } else {
            enemy.TailCut(static_cast<int>(i));
            if (!enemy.IsAlive()) {
                enemy_snake.Kill();
            }
        }
        break;
    }
}

bool game::IsRockCell(Vector2 position, bool solid_only) const {
    for (const FallingRock& rock : falling_rocks) {
        if ((!solid_only || rock.IsSolid()) &&
            Vector2Equals(rock.GetPosition(), position)) {
            return true;
        }
    }
    return false;
}

std::vector<bool> game::BuildBlockedCells() const {
    std::vector<bool> blocked_cells = wall_cells;
    for (const FallingRock& rock : falling_rocks) {
        if (!rock.IsSolid()) {
            continue;
        }
        Vector2 position = rock.GetPosition();
        int x = static_cast<int>(position.x);
        int y = static_cast<int>(position.y);
        if (x >= 0 && x < cellcount_width && y >= 0 && y < cellcount_height) {
            blocked_cells[y * cellcount_width + x] = true;
        }
    }
    return blocked_cells;
}

void game::InitGround() {
    ground_directions.clear();
    ground_directions.reserve(cellcount_width * cellcount_height);
    for (int i = 0; i < cellcount_width * cellcount_height; i++) {
        ground_directions.push_back(GetRandomValue(0, 1) == 0 ? dir_left : dir_right);
    }
}

void game::DrawGround() {
    if (game_sprite.id == 0) {
        return;
    }

    Rectangle ground_sprite = {16.0f, 16.0f, 16.0f, 16.0f};
    for (int y = 0; y < cellcount_height; y++) {
        for (int x = 0; x < cellcount_width; x++) {
            int cell_index = y * cellcount_width + x;
            float rotation = ground_directions[cell_index] == dir_left ? 180.0f : 0.0f;
            Rectangle draw_pos = {
                x * cellsize + cellsize / 2.0f,
                y * cellsize + cellsize / 2.0f,
                static_cast<float>(cellsize),
                static_cast<float>(cellsize)
            };
            Vector2 origin = {draw_pos.width / 2.0f, draw_pos.height / 2.0f};
            DrawTexturePro(game_sprite, ground_sprite, draw_pos, origin, rotation, WHITE);
        }
    }
}

void game::InitDoors() {
    door_positions.clear();
    constexpr int door_count = 5;
    for (int i = 0; i < door_count; i++) {
        float x = 4.0f + i * 8.0f;
        door_positions.push_back(Vector2{x, 0.0f});
    }
}

void game::DrawDoors() {
    if (game_sprite.id == 0) {
        return;
    }

    Rectangle source = {32.0f, 16.0f, 16.0f, 16.0f};
    for (size_t i = 0; i < door_positions.size(); i++) {
        Vector2 position = door_positions[i];
        Rectangle destination = {
            position.x * cellsize + cellsize / 2.0f,
            position.y * cellsize + cellsize / 2.0f,
            static_cast<float>(cellsize),
            static_cast<float>(cellsize)
        };
        Vector2 origin = {destination.width / 2.0f, destination.height / 2.0f};
        DrawTexturePro(game_sprite, source, destination, origin, 0.0f,
                       Color{185, 185, 195, 255});
    }
}

void game::DrawStageBanner() {
    if (!stage_progress.IsBannerVisible()) {
        return;
    }

    const char* stage_name = TextFormat("STAGE %d", stage_progress.GetStageIndex());
    int font_size = 34;
    int text_width = MeasureText(stage_name, font_size);
    int center_y = cellcount_height * cellsize / 2;
    unsigned char alpha = static_cast<unsigned char>(255.0f * stage_progress.GetBannerAlpha());
    Color background = {6, 13, 20, static_cast<unsigned char>(alpha * 0.82f)};
    Color foreground = {80, 225, 190, alpha};
    DrawRectangle(0, center_y - 34, screenWidth, 68, background);
    DrawText(stage_name, (screenWidth - text_width) / 2, center_y - font_size / 2,
             font_size, foreground);
}

void game::DrawUI() {
    int ui_y = cellcount_height * cellsize;
    Color ui_background = {8, 18, 27, 255};
    DrawRectangle(0, ui_y, screenWidth, screenHeight - ui_y, ui_background);
    DrawLineEx(Vector2{0.0f, static_cast<float>(ui_y)},
               Vector2{static_cast<float>(screenWidth), static_cast<float>(ui_y)},
               2.0f, darkGreen);

    int first_row_y = ui_y + 15;
    int second_row_y = ui_y + 45;
    DrawText(TextFormat("Score: %d", state.score), 16, first_row_y, 18, darkGreen);
    DrawText(TextFormat("Combo: %d", combo_counter), 180, first_row_y, 18, darkGreen);
    int multiplier_percent = static_cast<int>(score_multiplier * 100.0f + 0.5f);
    DrawText(TextFormat("Multiplier: %d%%", multiplier_percent), 330, first_row_y, 18, darkGreen);
    DrawText(TextFormat("Stage: %d", stage_progress.GetStageIndex()), 530, first_row_y, 18, darkGreen);

    if (stage_progress.IsKeyActive()) {
        DrawText("Key available - collect it to advance", 16, second_row_y, 18,
                 Color{236, 197, 74, 255});
    } else {
        DrawText(TextFormat("Food: %d / %d", stage_progress.GetAppleCount(),
                            stage_progress.GetFoodGoal()),
                 16, second_row_y, 18, Color{132, 200, 179, 255});
    }
}

void game::ShowScorePopup(Vector2 position, int value) {
    score_popup_position = position;
    score_popup_value = value;
    score_popup_remaining = score_popup_duration;
    score_popup_active = true;
}

void game::UpdateScorePopup() {
    if (!score_popup_active) {
        return;
    }

    score_popup_remaining -= GetFrameTime();
    if (score_popup_remaining <= 0.0f) {
        score_popup_remaining = 0.0f;
        score_popup_active = false;
    }
}

void game::DrawScorePopup() {
    if (!score_popup_active || score_popup_duration <= 0.0f) {
        return;
    }

    float remaining_ratio = score_popup_remaining / score_popup_duration;
    float animation_progress = 1.0f - remaining_ratio;
    const char* popup_text = TextFormat("%+d", score_popup_value);
    int font_size = 16;
    int text_width = MeasureText(popup_text, font_size);
    float draw_x = score_popup_position.x * cellsize + cellsize / 2.0f - text_width / 2.0f;
    float draw_y = score_popup_position.y * cellsize - animation_progress * 20.0f;
    if (draw_y < 2.0f) {
        draw_y = 2.0f;
    }

    Color popup_color = score_popup_value < 0 ? Color{218, 74, 66, 255} : darkGreen;
    popup_color.a = static_cast<unsigned char>(255.0f * remaining_ratio);
    DrawText(popup_text, static_cast<int>(draw_x), static_cast<int>(draw_y), font_size, popup_color);
}

void game::InitStationaryWall() {
    for (int x=0; x<cellcount_width; x++) {
        AddWallBrick(Vector2{(float)x, 0});
        AddWallBrick(Vector2{(float)x, (float)cellcount_height - 1});
    }

    for (int y=0; y<cellcount_height; y++) {
        AddWallBrick(Vector2{0, (float)y});
        AddWallBrick(Vector2{(float)cellcount_width - 1, (float)y});
    }
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

int game::GetFoodScoreWithCombo(int food_score) {
    return static_cast<int>(food_score * score_multiplier + 0.5f);
}
