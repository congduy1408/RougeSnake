#pragma once
#include "include/common.h"
#include "include/snake.h"
#include "include/food.h"
#include "include/wall.h"
#include "include/gamestate.h"
#include "include/board.h"
#include "include/room_template.h"
#include "include/room.h"
#include "include/game_textures.h"

class game {
    public:
        gamestate state;

        Board board{cellcount_width, cellcount_height};
        
        Snake spawn_snake;
        Food spawn_food;
        RoomTemplate room_template;
        Room room;
        
        int combo_counter = 0;
        int combo_score_step_count = 5;
        float score_multiplier = 1;
        float score_multiplier_start = 1.5;
        float score_multiplier_step = 0.5;
        float score_multiplier_max = 5;
        double last_get_time = 0.0;
    bool InitGameObject();
    void Draw(const GameTextures& textures);
    void ReadInput();
    void FixUpdate(float interval);

    // bool SnakeCollision(const Snake& snake, const GameObject& object) const;
    
    void HandleFoodCollision();
    void UpdateComboCounter(int food_score, int food_max_score);
    float GetFoodScoreWithCombo(int food_score);
};
