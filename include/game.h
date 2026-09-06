#include "include/common.h"
#include "include/audio_system.h"
#include "include/snake.h"
#include "include/enemy_snake.h"
#include "include/elite_enemy.h"
#include "include/first_boss.h"
#include "include/food.h"
#include "include/wall.h"
#include "include/gamestate.h"
#include "include/rock.h"
#include "include/stage.h"
#include "include/item.h"

#include <memory>

class game {
    public:
        explicit game(AudioSystem& audio_system, int debug_start_stage = 0);
        ~game();
        game(const game&) = delete;
        game& operator=(const game&) = delete;
        gamestate state;
        Snake spawn_snake;
        Food spawn_food;
        StageProgress stage_progress;
        std::vector<std::unique_ptr<EnemySnake>> common_enemies;
        std::unique_ptr<EliteEnemy> elite_enemy;
        std::unique_ptr<FirstBoss> boss_enemy;
        std::vector<Brick> wall_bricks;
        std::vector<bool> wall_cells;
        std::vector<FallingRock> falling_rocks;
        int combo_counter = 0;
        int combo_score_step_count = 5;
        float score_multiplier = 1;
        float score_multiplier_start = 1.5;
        float score_multiplier_step = 0.5;
        float score_multiplier_max = 5;
        float rock_warning_duration = 2.5f;
        float rock_fall_duration = 0.45f;
        float rock_lifetime = 20.0f;
        float rock_spawn_interval_min = 5.0f;
        float rock_spawn_interval_max = 10.0f;
        int max_falling_rocks = 10;
        int max_rocks_on_map = 20;
        float rock_screen_shake_duration = 0.6f;
        int rock_screen_shake_intensity = 3;
        int stage_three_common_enemy_count = 2;
        int boss_rock_enemy_drop_count = 2;
        int boss_rock_enemy_stage_max = 4;
        int elite_spawn_progress_percent = 75;
        int elite_min_length = 5;
        int elite_max_length = 10;
        int elite_random_turn_chance_percent = 15;
        int boss_random_turn_chance_percent = 20;
        int boss_attack_length_threshold = 7;
        int enemy_speed_min_percent = 85;
        int enemy_speed_max_percent = 125;
        int enemy_speed_boost_chance_percent = 5;
        float enemy_speed_boost_multiplier = 1.5f;
        int enemy_speed_boost_min_moves = 3;
        int enemy_speed_boost_max_moves = 6;
        int high_score_food_base_spawn_chance_percent = 10;
        int high_score_food_combo_size = 5;
        int high_score_food_combo_chance_percent = 10;
        int moving_food_base_spawn_chance_percent = 5;
        int moving_food_stage_three_spawn_chance_percent = 50;
        std::vector<ItemDefinition> item_definitions;
        double last_get_time = 0.0;
    void InitGameObject();
    void Draw();
    void Update();
    bool FixUpdate(float interval, double& last_update_time);
    bool SnakeCollision(Snake& snake, GameObject object);
    int CellIndex(Vector2 pos);
    bool IsWallCell(Vector2 pos);
    void AddWallBrick(Vector2 pos);
    void InitStationaryWall();
    void UpdateComboCounter(int food_score, int food_max_score);
    int GetFoodScoreWithCombo(int food_score);
    private:
        AudioSystem& audio;
        Texture2D game_sprite = {};
        int debug_start_stage = 0;
        int high_score = 0;
        std::string high_score_path;
        std::vector<direction> ground_directions;
        double next_rock_spawn_time = 0.0;
        std::vector<double> item_next_spawn_times;
        std::vector<ActiveItem> active_items;
        std::vector<ActiveItemEffect> active_item_effects;
        Vector2 screen_shake_offset = {};
        float screen_shake_remaining = 0.0f;
        bool elite_spawned = false;
        bool score_popup_active = false;
        Vector2 score_popup_position = {};
        int score_popup_value = 0;
        float score_popup_remaining = 0.0f;
        float score_popup_duration = 0.8f;
        void DrawCenteredText(const char* text, int y, int font_size, Color color);
        bool IsBossStage(int stage_index) const;
        int GetBossStageNumber(int stage_index) const;
        float GetSceneSpriteOffset() const;
        int CountActiveSupportEnemies() const;
        void LoadHighScore();
        void UpdateHighScore();
        void SaveHighScore();
        void InitGround();
        void DrawMainMenuBackground();
        void DrawItems();
        void DrawGround(float sprite_x_offset);
        void DrawStageBanner();
        void DrawUI();
        void ShowScorePopup(Vector2 position, int value);
        void UpdateScorePopup();
        void DrawScorePopup();
        int FindBodyCollisionIndex(const Snake& moving_snake, const Snake& target_snake);
        void DrawEnemies();
        bool UpdateEnemies(const std::vector<bool>& blocked_cells);
        void HandleCrossSnakeCollisions();
        void HandleCollisionWithEnemy(EnemySnake& enemy, bool drops_key);
        void HandleEnemyDeath(EnemySnake& enemy, bool killed_by_player, bool drops_key,
                              int reward_length = 0);
        void RewardEnemyKill(int enemy_length, Vector2 position);
        void DropStageKey(Vector2 position);
        bool SpawnCommonEnemy();
        void SpawnCommonEnemies(int count);
        bool SpawnEliteEnemy(bool trigger_rock_wave = true);
        void SpawnFirstBoss();
        void ConfigureEnemySpeed(EnemySnake& enemy);
        bool FindEnemySpawn(int length, direction start_direction, Vector2& position) const;
        bool IsAnySnakeCell(Vector2 position) const;
        void CheckEliteSpawnProgress();
        void SpawnBossRockEnemies();
        void ApplyPlayerCutPenalty(int removed_body_count, Vector2 popup_position);
        void HandlePlayerFoodCollision();
        void AdvanceStage();
        void UpdateRocks();
        void InitializeDefaultItems();
        void ResetItems();
        void UpdateItems();
        void DrawItem(const ActiveItem& item);
        void ScheduleNextItem(std::size_t definition_index);
        void TrySpawnItem(std::size_t definition_index);
        int GetItemSpawnChance(const ItemDefinition& definition) const;
        bool IsItemCell(Vector2 position) const;
        void CollectItems();
        void ApplyItemEffect(const ItemDefinition& definition);
        void UpdateItemEffects();
        bool HasItemEffect(ItemEffectType effect_type) const;
        float GetMovementSpeedMultiplier() const;
        bool SpawnRockWave(int maximum_wave_size);
        bool SpawnFallingItem(std::size_t definition_index);
        void LandFallingItem(const FallingRock& item_drop);
        void StartScreenShake();
        void UpdateScreenShake(float delta_time);
        bool SpawnRock();
        void ScheduleNextRock();
        void HandleRockImpact(Vector2 position);
        bool IsRockCell(Vector2 position, bool solid_only) const;
        std::vector<bool> BuildBlockedCells() const;
        bool ShouldSpawnHighScoreFood() const;
        bool ShouldSpawnMovingFood() const;
        void RespawnFood();
        void ApplyDebugStartStage();
};
