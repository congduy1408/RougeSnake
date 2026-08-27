#include "include/common.h"
#include "include/snake.h"
#include "include/enemy_snake.h"
#include "include/food.h"
#include "include/wall.h"
#include "include/gamestate.h"
#include "include/rock.h"
#include "include/stage.h"

class game {
    public:
        game();
        ~game();
        game(const game&) = delete;
        game& operator=(const game&) = delete;
        gamestate state;
        Snake spawn_snake;
        EnemySnake enemy_snake;
        Food spawn_food;
        StageProgress stage_progress;
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
        Texture2D game_sprite = {};
        int high_score = 0;
        std::string high_score_path;
        std::vector<direction> ground_directions;
        std::vector<Vector2> door_positions;
        double next_rock_spawn_time = 0.0;
        bool score_popup_active = false;
        Vector2 score_popup_position = {};
        int score_popup_value = 0;
        float score_popup_remaining = 0.0f;
        float score_popup_duration = 0.8f;
        void DrawCenteredText(const char* text, int y, int font_size, Color color);
        void LoadHighScore();
        void UpdateHighScore();
        void SaveHighScore();
        void InitGround();
        void DrawGround();
        void DrawDoors();
        void DrawStageBanner();
        void DrawUI();
        void ShowScorePopup(Vector2 position, int value);
        void UpdateScorePopup();
        void DrawScorePopup();
        int FindBodyCollisionIndex(const Snake& moving_snake, const Snake& target_snake);
        void HandleCrossSnakeCollisions();
        void ApplyPlayerCutPenalty(int removed_body_count, Vector2 popup_position);
        void HandlePlayerFoodCollision();
        void AdvanceStage();
        void InitDoors();
        void UpdateRocks();
        bool SpawnRock();
        void ScheduleNextRock();
        void HandleRockImpact(Vector2 position);
        bool IsRockCell(Vector2 position, bool solid_only) const;
        std::vector<bool> BuildBlockedCells() const;
        void RespawnFood();
};
