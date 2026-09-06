#include "include/game.h"

#include <algorithm>
#include <cstdio>

game::game(AudioSystem& audio_system, int debug_start_stage_value)
    : audio(audio_system) {
    debug_start_stage = debug_start_stage_value;
    game_sprite = LoadTexture("sprite/game_sprite.png");
    SetTextureFilter(game_sprite, TEXTURE_FILTER_POINT);
    spawn_food.SetSpriteTexture(game_sprite);
    InitializeDefaultItems();
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

bool game::IsBossStage(int stage_index) const {
    return stage_index >= 4 && (stage_index - 4) % 3 == 0;
}

int game::GetBossStageNumber(int stage_index) const {
    if (!IsBossStage(stage_index)) {
        return 0;
    }
    return ((stage_index - 4) / 3) + 1;
}

float game::GetSceneSpriteOffset() const {
    int stage_index = stage_progress.GetStageIndex();
    int completed_boss_count = stage_index >= 4 ? (stage_index - 2) / 3 : 0;
    if (IsBossStage(stage_index) && boss_enemy != nullptr && !boss_enemy->IsAlive()) {
        completed_boss_count++;
    }
    return completed_boss_count % 2 == 0 ? 0.0f : 48.0f;
}

int game::CountActiveSupportEnemies() const {
    int enemy_count = 0;
    for (const std::unique_ptr<EnemySnake>& enemy : common_enemies) {
        if (enemy->IsAlive()) {
            enemy_count++;
        }
    }
    if (elite_enemy != nullptr && elite_enemy->IsAlive()) {
        enemy_count++;
    }
    return enemy_count;
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
    common_enemies.clear();
    elite_enemy.reset();
    boss_enemy.reset();
    elite_spawned = false;
    stage_progress.Reset();
    falling_rocks.clear();
    next_rock_spawn_time = 0.0;
    ResetItems();
    screen_shake_offset = Vector2{0.0f, 0.0f};
    screen_shake_remaining = 0.0f;
    InitGround();
    wall_bricks.clear();
    wall_cells.assign(cellcount_width * cellcount_height, false);
    InitStationaryWall();
    spawn_food.Reset(FoodType::Apple);
    RespawnFood();
    ApplyDebugStartStage();
    audio.PlayMusic(state.currentScreen == MAIN_MENU ? MusicId::MainMenu : MusicId::Gameplay);
    last_get_time = GetTime();
}

void game::ApplyDebugStartStage() {
    if (debug_start_stage <= 0) {
        return;
    }

    while (stage_progress.GetStageIndex() < debug_start_stage) {
        AdvanceStage();
    }
}

void game::Draw() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            DrawMainMenuBackground();
            DrawCenteredText("ROUGE SNAKE", screenHeight / 2 - 90, 40,
                             Color{218, 74, 66, 255});
            DrawCenteredText(TextFormat("High Score: %d", high_score),
                             screenHeight / 2 - 20, 24, Color{236, 197, 74, 255});
            if (static_cast<int>(GetTime() * 2.0) % 2 == 0)
            {
                DrawCenteredText("Press any key to start", screenHeight / 2 + 35,
                                 20, Color{132, 200, 179, 255});
            }
        } break;
        case STAGE: {
            Camera2D shake_camera = {};
            shake_camera.offset = screen_shake_offset;
            shake_camera.zoom = 1.0f;
            BeginMode2D(shake_camera);
            float tile_sprite_offset = GetSceneSpriteOffset();
            DrawGround(tile_sprite_offset);
            for (unsigned int i=0; i<wall_bricks.size(); i++) {
                wall_bricks[i].Draw(tile_sprite_offset);
            }
            spawn_snake.Draw();
            DrawEnemies();
            spawn_food.Draw();
            DrawItems();
            for (const FallingRock& rock : falling_rocks) {
                rock.Draw(tile_sprite_offset);
            }
            DrawScorePopup();
            DrawStageBanner();
            DrawUI();
            EndMode2D();
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
            if (GetKeyPressed() != 0)
            {
                state.currentScreen = STAGE;
                audio.PlayMusic(IsBossStage(stage_progress.GetStageIndex())
                    ? MusicId::Boss
                    : MusicId::Gameplay);
            }
        } break;
        case STAGE: {
            stage_progress.Update(GetFrameTime());
            UpdateScorePopup();
            UpdateRocks();
            if (state.currentScreen == GAMEOVER) {
                break;
            }
            UpdateItems();
            spawn_food.Update(BuildBlockedCells());
            if (!spawn_snake.IsAlive()) {
                state.currentScreen = GAMEOVER;
                break;
            }
            float speed_multiplier = GetMovementSpeedMultiplier();
            if (IsKeyDown(KEY_X)) {
                speed_multiplier = std::max(speed_multiplier, 1.5f);
            }
            spawn_snake.SetMovementSpeedMultiplier(speed_multiplier);
            spawn_snake.ReadInput(HasItemEffect(ItemEffectType::InvertControls),
                                  HasItemEffect(ItemEffectType::ReverseDirection));
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
            bool enemy_moved = UpdateEnemies(blocked_cells);

            if (player_moved || enemy_moved) {
                HandleCrossSnakeCollisions();
                if (state.currentScreen == GAMEOVER) {
                    break;
                }
            }

            if (player_moved) {
                CollectItems();
                if (SnakeCollision(spawn_snake, spawn_food)) {
                    HandlePlayerFoodCollision();
                } else {
                    if (spawn_food.GetFoodType() != FoodType::Key &&
                        spawn_food.UpdateBoundaryScore(spawn_snake.body.front().position)) {
                        combo_counter = 0;
                        score_multiplier = 1;
                    }
                }
            }

        } break;
        case GAMEOVER: {
            UpdateHighScore();
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
            {
                
                InitGameObject();
                state.currentScreen = STAGE;
                audio.PlayMusic(MusicId::Gameplay);
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

void game::DrawEnemies() {
    for (const std::unique_ptr<EnemySnake>& enemy : common_enemies) {
        enemy->Draw();
    }
    if (elite_enemy != nullptr) {
        elite_enemy->Draw();
    }
    if (boss_enemy != nullptr) {
        boss_enemy->Draw();
    }
}

bool game::UpdateEnemies(const std::vector<bool>& blocked_cells) {
    bool any_enemy_moved = false;
    auto update_enemy = [&](EnemySnake& enemy, bool drops_key) {
        if (!enemy.IsAlive()) {
            return;
        }

        FirstBoss* boss = dynamic_cast<FirstBoss*>(&enemy);
        if (boss != nullptr) {
            boss->SetTargets(spawn_snake.body.front().position, spawn_food.GetPosition(), true);
        } else {
            enemy.SetTarget(spawn_food.GetPosition(), true);
        }
        bool moved = enemy.Update(blocked_cells);
        any_enemy_moved = any_enemy_moved || moved;
        if (!enemy.IsAlive()) {
            HandleEnemyDeath(enemy, false, drops_key);
            return;
        }
        if (moved && spawn_food.GetFoodType() != FoodType::Key &&
            SnakeCollision(enemy.GetSnake(), spawn_food)) {
            enemy.GetSnake().Grow();
            RespawnFood();
        }
    };

    for (const std::unique_ptr<EnemySnake>& enemy : common_enemies) {
        update_enemy(*enemy, false);
    }
    if (elite_enemy != nullptr) {
        update_enemy(*elite_enemy, !IsBossStage(stage_progress.GetStageIndex()));
    }
    if (boss_enemy != nullptr) {
        update_enemy(*boss_enemy, true);
    }
    return any_enemy_moved;
}

void game::HandleCrossSnakeCollisions() {
    if (spawn_snake.body.empty()) {
        return;
    }

    for (const std::unique_ptr<EnemySnake>& enemy : common_enemies) {
        HandleCollisionWithEnemy(*enemy, false);
    }
    if (elite_enemy != nullptr) {
        HandleCollisionWithEnemy(*elite_enemy,
                                 !IsBossStage(stage_progress.GetStageIndex()));
    }
    if (boss_enemy != nullptr) {
        HandleCollisionWithEnemy(*boss_enemy, true);
    }
    if (!spawn_snake.IsAlive()) {
        state.currentScreen = GAMEOVER;
    }
}

void game::HandleCollisionWithEnemy(EnemySnake& enemy_object, bool drops_key) {
    if (!enemy_object.IsAlive() || spawn_snake.body.empty()) {
        return;
    }

    Snake& enemy = enemy_object.GetSnake();
    Vector2 enemy_head_position = enemy.body.front().position;
    int enemy_length_before_hit = static_cast<int>(enemy.body.size());
    int enemy_cut_index = FindBodyCollisionIndex(spawn_snake, enemy);
    int player_cut_index = FindBodyCollisionIndex(enemy, spawn_snake);

    if (player_cut_index > 0) {
        int removed_body_count = spawn_snake.TailCut(player_cut_index);
        ApplyPlayerCutPenalty(removed_body_count, enemy_head_position);
    }
    if (enemy_cut_index > 0) {
        enemy.TailCut(enemy_cut_index);
        if (!enemy.IsAlive()) {
            HandleEnemyDeath(enemy_object, true, drops_key, enemy_length_before_hit);
        }
    }
}

void game::HandleEnemyDeath(EnemySnake& enemy, bool killed_by_player, bool drops_key,
                            int reward_length) {
    int death_length = enemy.GetSnake().body.empty()
        ? enemy.GetLastDeathLength()
        : static_cast<int>(enemy.GetSnake().body.size());
    Vector2 death_position = enemy.GetSnake().body.empty()
        ? enemy.GetLastDeathPosition()
        : enemy.GetSnake().body.front().position;
    enemy.Kill();

    if (killed_by_player) {
        RewardEnemyKill(reward_length > 0 ? reward_length : death_length, death_position);
    }
    if (drops_key && !stage_progress.IsKeyActive()) {
        DropStageKey(death_position);
    }
}

void game::RewardEnemyKill(int enemy_length, Vector2 position) {
    if (enemy_length <= 0) {
        return;
    }
    int reward = enemy_length * 3;
    state.score += reward;
    ShowScorePopup(position, reward);
}

void game::DropStageKey(Vector2 position) {
    stage_progress.ActivateKey();
    spawn_food.Reset(FoodType::Key);
    if (!IsWallCell(position) && !IsAnySnakeCell(position)) {
        spawn_food.SetPosition(position);
    } else {
        RespawnFood();
    }
}

bool game::SpawnCommonEnemy() {
    direction start_direction = GetRandomDirection();
    Vector2 spawn_position = {};
    if (!FindEnemySpawn(3, start_direction, spawn_position)) {
        return false;
    }
    std::unique_ptr<EnemySnake> enemy(new EnemySnake());
    enemy->Reset(spawn_position, start_direction, 3);
    ConfigureEnemySpeed(*enemy);
    common_enemies.push_back(std::move(enemy));
    return true;
}

void game::SpawnCommonEnemies(int count) {
    for (int i = 0; i < count; i++) {
        if (!SpawnCommonEnemy()) {
            continue;
        }
    }
}

bool game::SpawnEliteEnemy(bool trigger_rock_wave) {
    if (elite_enemy != nullptr && elite_enemy->IsAlive()) {
        return false;
    }

    int minimum_length = elite_min_length;
    int maximum_length = elite_max_length;
    if (maximum_length < minimum_length) {
        std::swap(minimum_length, maximum_length);
    }
    if (minimum_length < 3) {
        minimum_length = 3;
    }
    int elite_length = GetRandomValue(minimum_length, maximum_length);
    direction start_direction = GetRandomDirection();
    Vector2 spawn_position = {};
    if (!FindEnemySpawn(elite_length, start_direction, spawn_position)) {
        return false;
    }

    elite_enemy.reset(new EliteEnemy());
    elite_enemy->random_turn_chance_percent = elite_random_turn_chance_percent;
    elite_enemy->Reset(spawn_position, start_direction, elite_length);
    ConfigureEnemySpeed(*elite_enemy);
    elite_spawned = true;
    if (trigger_rock_wave) {
        SpawnRockWave(max_falling_rocks);
    }
    return true;
}

void game::SpawnFirstBoss() {
    direction start_direction = GetRandomDirection();
    Vector2 spawn_position = {};
    constexpr int boss_length = 10;
    if (!FindEnemySpawn(boss_length, start_direction, spawn_position)) {
        return;
    }

    boss_enemy.reset(new FirstBoss());
    boss_enemy->random_turn_chance_percent = boss_random_turn_chance_percent;
    boss_enemy->attack_length_threshold = boss_attack_length_threshold;
    boss_enemy->Reset(spawn_position, start_direction, boss_length);
    ConfigureEnemySpeed(*boss_enemy);
}

void game::ConfigureEnemySpeed(EnemySnake& enemy) {
    if (stage_progress.GetStageIndex() < 5) {
        return;
    }

    int minimum_percent = enemy_speed_min_percent;
    int maximum_percent = enemy_speed_max_percent;
    if (maximum_percent < minimum_percent) {
        std::swap(minimum_percent, maximum_percent);
    }
    if (minimum_percent < 1) {
        minimum_percent = 1;
    }
    if (maximum_percent < 1) {
        maximum_percent = 1;
    }
    float base_multiplier = GetRandomValue(minimum_percent, maximum_percent) / 100.0f;
    enemy.ConfigureMovementSpeed(base_multiplier, enemy_speed_boost_chance_percent,
                                 enemy_speed_boost_multiplier, enemy_speed_boost_min_moves,
                                 enemy_speed_boost_max_moves);
}

bool game::FindEnemySpawn(int length, direction start_direction, Vector2& position) const {
    constexpr int max_attempts = 200;
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        Vector2 candidate = {
            static_cast<float>(GetRandomValue(1, cellcount_width - 2)),
            static_cast<float>(GetRandomValue(1, cellcount_height - 2))
        };
        bool blocked = false;
        for (int i = 0; i < length; i++) {
            Vector2 body_position = candidate;
            switch (start_direction) {
                case dir_up: body_position.y += i; break;
                case dir_down: body_position.y -= i; break;
                case dir_left: body_position.x += i; break;
                case dir_right: body_position.x -= i; break;
                default: break;
            }
            int x = static_cast<int>(body_position.x);
            int y = static_cast<int>(body_position.y);
            int cell_index = y * cellcount_width + x;
            if (x <= 0 || x >= cellcount_width - 1 || y <= 0 ||
                y >= cellcount_height - 1 || cell_index < 0 ||
                static_cast<size_t>(cell_index) >= wall_cells.size() ||
                wall_cells[cell_index] || IsRockCell(body_position, false) ||
                IsAnySnakeCell(body_position)) {
                blocked = true;
                break;
            }
        }
        if (!blocked) {
            position = candidate;
            return true;
        }
    }
    return false;
}

bool game::IsAnySnakeCell(Vector2 position) const {
    for (const snake_body& body_part : spawn_snake.body) {
        if (Vector2Equals(body_part.position, position)) {
            return true;
        }
    }
    auto contains_position = [&](const EnemySnake& enemy) {
        if (!enemy.IsAlive()) {
            return false;
        }
        for (const snake_body& body_part : enemy.GetSnake().body) {
            if (Vector2Equals(body_part.position, position)) {
                return true;
            }
        }
        return false;
    };
    for (const std::unique_ptr<EnemySnake>& enemy : common_enemies) {
        if (contains_position(*enemy)) {
            return true;
        }
    }
    return (elite_enemy != nullptr && contains_position(*elite_enemy)) ||
           (boss_enemy != nullptr && contains_position(*boss_enemy));
}

void game::CheckEliteSpawnProgress() {
    int stage_index = stage_progress.GetStageIndex();
    if (stage_index < 3 || IsBossStage(stage_index) || elite_spawned) {
        return;
    }
    int required_percent = elite_spawn_progress_percent;
    if (required_percent < 0) {
        required_percent = 0;
    } else if (required_percent > 100) {
        required_percent = 100;
    }
    if (stage_progress.GetAppleCount() * 100 >=
        stage_progress.GetFoodGoal() * required_percent) {
        SpawnEliteEnemy();
    }
}

void game::SpawnBossRockEnemies() {
    int stage_index = stage_progress.GetStageIndex();
    int boss_stage_number = GetBossStageNumber(stage_index);
    if (boss_stage_number <= 0) {
        return;
    }

    int maximum_enemy_count = boss_rock_enemy_stage_max;
    if (maximum_enemy_count < 0) {
        maximum_enemy_count = 0;
    }
    int available_slots = maximum_enemy_count - CountActiveSupportEnemies();
    if (available_slots <= 0) {
        return;
    }

    int requested_drop_count = boss_rock_enemy_drop_count;
    if (requested_drop_count < 0) {
        requested_drop_count = 0;
    }
    int drop_count = requested_drop_count < available_slots ? requested_drop_count : available_slots;
    bool can_spawn_elites = boss_stage_number >= 2;

    for (int i = 0; i < drop_count; i++) {
        bool spawned = false;
        if (can_spawn_elites && (elite_enemy == nullptr || !elite_enemy->IsAlive()) &&
            GetRandomValue(0, 1) == 1) {
            spawned = SpawnEliteEnemy(false);
        }
        if (!spawned) {
            SpawnCommonEnemy();
        }
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
    UpdateComboCounter(food_score, spawn_food.GetMaxScore());
    int gained_score = GetFoodScoreWithCombo(food_score);
    state.score += gained_score;
    ShowScorePopup(eaten_position, gained_score);
    audio.PlaySoundEffect(SoundEffectId::FoodEaten);
    spawn_snake.Grow();

    bool use_food_key = stage_progress.GetStageIndex() < 3;
    if (stage_progress.RecordAppleEaten(use_food_key)) {
        spawn_food.Reset(FoodType::Key);
    } else {
        spawn_food.Reset(FoodType::Apple);
    }
    CheckEliteSpawnProgress();
    RespawnFood();
}

void game::AdvanceStage() {
    stage_progress.Advance();
    int stage_index = stage_progress.GetStageIndex();
    common_enemies.clear();
    elite_enemy.reset();
    boss_enemy.reset();
    elite_spawned = false;
    spawn_food.Reset(FoodType::Apple);
    combo_counter = 0;
    score_multiplier = 1.0f;

    if (stage_index == 1) {
        ScheduleNextRock();
    }
    if (stage_index == 2) {
        SpawnCommonEnemies(1);
    }
    if (stage_index == 3) {
        SpawnCommonEnemies(stage_three_common_enemy_count);
    }
    if (IsBossStage(stage_index)) {
        SpawnFirstBoss();
    } else if (stage_index > 4) {
        int cycle_stage = (stage_index - 2) % 3;
        if (cycle_stage == 0) {
            SpawnCommonEnemies(1);
        } else if (cycle_stage == 1) {
            SpawnCommonEnemies(stage_three_common_enemy_count);
        }
    }
    audio.PlayMusic(IsBossStage(stage_index) ? MusicId::Boss : MusicId::Gameplay);
    RespawnFood();
}

void game::RespawnFood() {
    if (spawn_food.GetFoodType() != FoodType::Key) {
        FoodType next_food_type = FoodType::Apple;
        if (ShouldSpawnHighScoreFood()) {
            next_food_type = FoodType::HighScore;
        } else if (ShouldSpawnMovingFood()) {
            next_food_type = FoodType::Moving;
        }
        spawn_food.Reset(next_food_type);
    }
    std::vector<bool> blocked_cells = BuildBlockedCells();
    spawn_food.SetFoodPosition(spawn_snake, blocked_cells);
}

bool game::ShouldSpawnHighScoreFood() const {
    int chance = high_score_food_base_spawn_chance_percent;
    if (high_score_food_combo_size > 0 && combo_counter > 0) {
        chance += (combo_counter / high_score_food_combo_size) *
                  high_score_food_combo_chance_percent;
    }
    chance = std::max(0, std::min(chance, 100));
    return GetRandomValue(1, 100) <= chance;
}

bool game::ShouldSpawnMovingFood() const {
    int chance = stage_progress.GetStageIndex() >= 3
        ? moving_food_stage_three_spawn_chance_percent
        : moving_food_base_spawn_chance_percent;
    chance = std::max(0, std::min(chance, 100));
    return GetRandomValue(1, 100) <= chance;
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
        if (rock.ShouldStartShake()) {
            StartScreenShake();
        }
        if (rock.Update(delta_time)) {
            if (rock.IsItemDrop()) {
                LandFallingItem(rock);
            } else {
                HandleRockImpact(rock.GetPosition());
            }
        }
    }
    UpdateScreenShake(delta_time);

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

    if (!SpawnRockWave(max_falling_rocks)) {
        return;
    }
    ScheduleNextRock();
}

void game::InitializeDefaultItems() {
    ItemDefinition speed_boost;
    speed_boost.effect = ItemEffectType::SpeedBoost;
    speed_boost.sprite_source = Rectangle{48.0f, 32.0f, 16.0f, 16.0f};
    speed_boost.effect_duration = -1.0f;
    speed_boost.falls_with_rocks = true;
    item_definitions.push_back(speed_boost);

    ItemDefinition speed_slow;
    speed_slow.effect = ItemEffectType::SpeedSlow;
    speed_slow.sprite_source = Rectangle{0.0f, 48.0f, 16.0f, 16.0f};
    speed_slow.effect_duration = -1.0f;
    speed_slow.effect_value = 0.5f;
    speed_slow.falls_with_rocks = true;
    item_definitions.push_back(speed_slow);

    ItemDefinition invert_controls;
    invert_controls.effect = ItemEffectType::InvertControls;
    invert_controls.sprite_source = Rectangle{16.0f, 48.0f, 16.0f, 16.0f};
    invert_controls.effect_duration = 10.0f;
    item_definitions.push_back(invert_controls);

    ItemDefinition reverse_direction;
    reverse_direction.effect = ItemEffectType::ReverseDirection;
    reverse_direction.sprite_source = Rectangle{32.0f, 48.0f, 16.0f, 16.0f};
    reverse_direction.effect_duration = -1.0f;
    item_definitions.push_back(reverse_direction);
}

void game::ResetItems() {
    active_items.clear();
    active_item_effects.clear();
    item_next_spawn_times.assign(item_definitions.size(), 0.0);
    for (std::size_t i = 0; i < item_definitions.size(); i++) {
        ScheduleNextItem(i);
    }
}

void game::UpdateItems() {
    if (item_next_spawn_times.size() != item_definitions.size()) {
        ResetItems();
    }

    double current_time = GetTime();
    active_items.erase(
        std::remove_if(active_items.begin(), active_items.end(),
            [current_time](const ActiveItem& item) { return current_time >= item.expire_at; }),
        active_items.end());
    UpdateItemEffects();

    for (std::size_t i = 0; i < item_definitions.size(); i++) {
        if (item_definitions[i].falls_with_rocks) {
            continue;
        }
        bool already_spawned = std::any_of(active_items.begin(), active_items.end(),
            [i](const ActiveItem& item) { return item.definition_index == i; });
        if (!already_spawned && current_time >= item_next_spawn_times[i]) {
            TrySpawnItem(i);
            ScheduleNextItem(i);
        }
    }
}

void game::ScheduleNextItem(std::size_t definition_index) {
    if (definition_index >= item_definitions.size() ||
        definition_index >= item_next_spawn_times.size()) {
        return;
    }
    float interval = std::max(item_definitions[definition_index].spawn_interval, 0.1f);
    item_next_spawn_times[definition_index] = GetTime() + interval;
}

int game::GetItemSpawnChance(const ItemDefinition& definition) const {
    int chance = definition.base_spawn_chance_percent;
    if (definition.combo_size > 0 && combo_counter > 0) {
        chance += (combo_counter / definition.combo_size) * definition.combo_spawn_chance_percent;
    }
    return std::max(0, std::min(chance, 100));
}

bool game::IsItemCell(Vector2 position) const {
    return std::any_of(active_items.begin(), active_items.end(),
        [position](const ActiveItem& item) { return Vector2Equals(item.position, position); });
}

void game::TrySpawnItem(std::size_t definition_index) {
    if (definition_index >= item_definitions.size() ||
        GetRandomValue(1, 100) > GetItemSpawnChance(item_definitions[definition_index])) {
        return;
    }

    constexpr int max_spawn_attempts = 100;
    for (int attempt = 0; attempt < max_spawn_attempts; attempt++) {
        Vector2 position = {
            static_cast<float>(GetRandomValue(1, cellcount_width - 2)),
            static_cast<float>(GetRandomValue(1, cellcount_height - 2))
        };
        if (IsWallCell(position) || IsRockCell(position, false) || IsAnySnakeCell(position) ||
            IsItemCell(position) || Vector2Equals(position, spawn_food.GetPosition())) {
            continue;
        }

        float lifetime = std::max(item_definitions[definition_index].pickup_expire_time, 0.1f);
        active_items.push_back(ActiveItem{definition_index, position, GetTime() + lifetime});
        return;
    }
}

void game::CollectItems() {
    if (spawn_snake.body.empty()) {
        return;
    }

    Vector2 player_position = spawn_snake.body.front().position;
    for (std::vector<ActiveItem>::iterator item = active_items.begin();
         item != active_items.end();) {
        if (!Vector2Equals(item->position, player_position)) {
            ++item;
            continue;
        }
        if (item->definition_index < item_definitions.size()) {
            ApplyItemEffect(item_definitions[item->definition_index]);
            audio.PlaySoundEffect(SoundEffectId::ItemCollected);
        }
        item = active_items.erase(item);
    }
}

void game::ApplyItemEffect(const ItemDefinition& definition) {
    ActiveItemEffect effect;
    effect.type = definition.effect;
    effect.value = definition.effect_value;
    effect.never_expires = definition.effect_duration < 0.0f;
    effect.expire_at = effect.never_expires ? 0.0 : GetTime() + definition.effect_duration;

    for (ActiveItemEffect& active_effect : active_item_effects) {
        if (active_effect.type == effect.type) {
            active_effect = effect;
            return;
        }
    }
    active_item_effects.push_back(effect);
}

void game::UpdateItemEffects() {
    double current_time = GetTime();
    active_item_effects.erase(
        std::remove_if(active_item_effects.begin(), active_item_effects.end(),
            [current_time](const ActiveItemEffect& effect) {
                return !effect.never_expires && current_time >= effect.expire_at;
            }),
        active_item_effects.end());
}

bool game::HasItemEffect(ItemEffectType effect_type) const {
    return std::any_of(active_item_effects.begin(), active_item_effects.end(),
        [effect_type](const ActiveItemEffect& effect) { return effect.type == effect_type; });
}

float game::GetMovementSpeedMultiplier() const {
    float multiplier = 1.0f;
    for (const ActiveItemEffect& effect : active_item_effects) {
        if (effect.type == ItemEffectType::SpeedBoost) {
            multiplier *= effect.value;
        } else if (effect.type == ItemEffectType::SpeedSlow) {
            multiplier *= effect.value;
        }
    }
    return multiplier;
}

bool game::SpawnRockWave(int maximum_wave_size) {
    int falling_count = 0;
    for (const FallingRock& rock : falling_rocks) {
        if (rock.GetState() == RockState::Shaking ||
            rock.GetState() == RockState::Warning ||
            rock.GetState() == RockState::Falling) {
            falling_count++;
        }
    }
    int falling_capacity = max_falling_rocks - falling_count;
    int map_capacity = max_rocks_on_map - static_cast<int>(falling_rocks.size());
    int wave_capacity = falling_capacity < map_capacity ? falling_capacity : map_capacity;
    if (maximum_wave_size < wave_capacity) {
        wave_capacity = maximum_wave_size;
    }
    if (wave_capacity <= 0) {
        return false;
    }

    int wave_size = GetRandomValue(1, wave_capacity);
    bool spawned_rock = false;
    for (int i = 0; i < wave_size; i++) {
        if (!SpawnRock()) {
            break;
        }
        spawned_rock = true;
        for (std::size_t item_index = 0; item_index < item_definitions.size(); item_index++) {
            const ItemDefinition& item = item_definitions[item_index];
            if (item.falls_with_rocks &&
                GetRandomValue(1, 100) <= GetItemSpawnChance(item)) {
                SpawnFallingItem(item_index);
            }
        }
    }
    return spawned_rock;
}

void game::StartScreenShake() {
    if (rock_screen_shake_duration > screen_shake_remaining) {
        screen_shake_remaining = rock_screen_shake_duration;
    }
}

void game::UpdateScreenShake(float delta_time) {
    if (screen_shake_remaining <= 0.0f || rock_screen_shake_intensity <= 0) {
        screen_shake_remaining = 0.0f;
        screen_shake_offset = Vector2{0.0f, 0.0f};
        return;
    }

    screen_shake_remaining -= delta_time;
    if (screen_shake_remaining <= 0.0f) {
        screen_shake_remaining = 0.0f;
        screen_shake_offset = Vector2{0.0f, 0.0f};
        return;
    }
    screen_shake_offset = Vector2{
        static_cast<float>(GetRandomValue(-rock_screen_shake_intensity,
                                          rock_screen_shake_intensity)),
        static_cast<float>(GetRandomValue(-rock_screen_shake_intensity,
                                          rock_screen_shake_intensity))
    };
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
        if (wall_cells[cell_index] || IsRockCell(position, false) || IsItemCell(position)) {
            continue;
        }

        falling_rocks.emplace_back(position, game_sprite, rock_screen_shake_duration,
                                   rock_warning_duration, rock_fall_duration, rock_lifetime);
        return true;
    }
    return false;
}

bool game::SpawnFallingItem(std::size_t definition_index) {
    if (definition_index >= item_definitions.size() ||
        std::any_of(active_items.begin(), active_items.end(),
            [definition_index](const ActiveItem& item) {
                return item.definition_index == definition_index;
            }) ||
        std::any_of(falling_rocks.begin(), falling_rocks.end(),
            [definition_index](const FallingRock& falling_object) {
                return falling_object.IsItemDrop() &&
                       falling_object.GetItemDefinitionIndex() == definition_index;
            })) {
        return false;
    }

    constexpr int max_spawn_attempts = 100;
    for (int attempt = 0; attempt < max_spawn_attempts; attempt++) {
        Vector2 position = {
            static_cast<float>(GetRandomValue(1, cellcount_width - 2)),
            static_cast<float>(GetRandomValue(1, cellcount_height - 2))
        };
        if (IsWallCell(position) || IsRockCell(position, false) || IsAnySnakeCell(position) ||
            IsItemCell(position) || Vector2Equals(position, spawn_food.GetPosition())) {
            continue;
        }

        const ItemDefinition& item = item_definitions[definition_index];
        falling_rocks.emplace_back(position, game_sprite, item.sprite_source, definition_index,
                                   rock_screen_shake_duration, rock_warning_duration,
                                   rock_fall_duration);
        return true;
    }
    return false;
}

void game::LandFallingItem(const FallingRock& item_drop) {
    std::size_t definition_index = item_drop.GetItemDefinitionIndex();
    if (definition_index >= item_definitions.size() || IsItemCell(item_drop.GetPosition())) {
        return;
    }

    float lifetime = std::max(item_definitions[definition_index].pickup_expire_time, 0.1f);
    active_items.push_back(ActiveItem{definition_index, item_drop.GetPosition(),
                                      GetTime() + lifetime});
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
    audio.PlaySoundEffect(SoundEffectId::RockImpact);
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

    auto hit_enemy = [&](EnemySnake& enemy_object, bool drops_key) {
        if (!enemy_object.IsAlive()) {
            return;
        }
        Snake& enemy = enemy_object.GetSnake();
        for (size_t i = 0; i < enemy.body.size(); i++) {
            if (!Vector2Equals(enemy.body[i].position, position)) {
                continue;
            }
            if (i == 0) {
                enemy_object.Kill();
                HandleEnemyDeath(enemy_object, false, drops_key);
            } else {
                enemy.TailCut(static_cast<int>(i));
                if (!enemy.IsAlive()) {
                    HandleEnemyDeath(enemy_object, false, drops_key);
                }
            }
            break;
        }
    };
    for (const std::unique_ptr<EnemySnake>& enemy : common_enemies) {
        hit_enemy(*enemy, false);
    }
    if (elite_enemy != nullptr) {
        hit_enemy(*elite_enemy, !IsBossStage(stage_progress.GetStageIndex()));
    }
    if (boss_enemy != nullptr) {
        hit_enemy(*boss_enemy, true);
    }

    SpawnBossRockEnemies();
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
    auto block_enemy = [&](const EnemySnake& enemy) {
        if (!enemy.IsAlive()) {
            return;
        }
        for (const snake_body& body_part : enemy.GetSnake().body) {
            int x = static_cast<int>(body_part.position.x);
            int y = static_cast<int>(body_part.position.y);
            if (x >= 0 && x < cellcount_width && y >= 0 && y < cellcount_height) {
                blocked_cells[y * cellcount_width + x] = true;
            }
        }
    };
    for (const std::unique_ptr<EnemySnake>& enemy : common_enemies) {
        block_enemy(*enemy);
    }
    if (elite_enemy != nullptr) {
        block_enemy(*elite_enemy);
    }
    if (boss_enemy != nullptr) {
        block_enemy(*boss_enemy);
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

void game::DrawMainMenuBackground() {
    if (game_sprite.id == 0) {
        return;
    }

    // Main-menu artwork occupies the 128x96 region at (0, 160) in the sprite sheet.
    Rectangle source = {0.0f, 160.0f, 128.0f, 96.0f};
    Rectangle destination = {0.0f, 0.0f,
                             static_cast<float>(screenWidth),
                             static_cast<float>(screenHeight)};
    DrawTexturePro(game_sprite, source, destination, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}

void game::DrawItems() {
    for (const ActiveItem& item : active_items) {
        DrawItem(item);
    }
}

void game::DrawItem(const ActiveItem& item) {
    if (game_sprite.id == 0 || item.definition_index >= item_definitions.size()) {
        return;
    }

    const ItemDefinition& definition = item_definitions[item.definition_index];
    Rectangle destination = {
        item.position.x * cellsize + cellsize / 2.0f,
        item.position.y * cellsize + cellsize / 2.0f,
        static_cast<float>(cellsize),
        static_cast<float>(cellsize)
    };
    Vector2 origin = {destination.width / 2.0f, destination.height / 2.0f};
    DrawTexturePro(game_sprite, definition.sprite_source, destination, origin, 0.0f, WHITE);
}

void game::DrawGround(float sprite_x_offset) {
    if (game_sprite.id == 0) {
        return;
    }

    Rectangle ground_sprite = {16.0f + sprite_x_offset, 16.0f, 16.0f, 16.0f};
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

void game::DrawStageBanner() {
    if (!stage_progress.IsBannerVisible()) {
        return;
    }

    int stage_index = stage_progress.GetStageIndex();
    const char* stage_name = IsBossStage(stage_index)
        ? TextFormat("BOSS %d", GetBossStageNumber(stage_index))
        : TextFormat("STAGE %d", stage_index);
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
    DrawText("Press X for speed up", 390, second_row_y, 18,
             Color{132, 200, 179, 255});
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
