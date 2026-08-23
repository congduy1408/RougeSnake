#include "include/common.h"
#include "include/snake.h"
#include "include/food.h"
#include "include/wall.h"
#include "include/gamestate.h"

class game {
    public:
        game();
        ~game();
        game(const game&) = delete;
        game& operator=(const game&) = delete;
        gamestate state;
        Snake spawn_snake;
        Food spawn_food;
        std::vector<Brick> wall_bricks;
        std::vector<bool> wall_cells;
        int combo_counter = 0;
        int combo_score_step_count = 5;
        float score_multiplier = 1;
        float score_multiplier_start = 1.5;
        float score_multiplier_step = 0.5;
        float score_multiplier_max = 5;
        double last_get_time = 0.0;
    void InitGameObject();
    void Draw();
    void Update();
    bool FixUpdate(float interval);
    bool SnakeCollision(Snake& snake, GameObject object);
    int CellIndex(Vector2 pos);
    bool IsWallCell(Vector2 pos);
    void AddWallBrick(Vector2 pos);
    void InitStationaryWall();
    void UpdateComboCounter(int food_score, int food_max_score);
    float GetFoodScoreWithCombo(int food_score);
    private:
        Texture2D game_sprite = {};
        std::vector<direction> ground_directions;
        void InitGround();
        void DrawGround();
        void DrawUI();
};
