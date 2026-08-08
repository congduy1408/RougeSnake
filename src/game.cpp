#include "include/game.h"

void game::InitGameObject() {
    state = gamestate();
    spawn_snake = snake();
    spawn_food = food();
    spawn_food.SetSnake(spawn_snake.body);
    spawn_food.SetFoodPosition();
}
void game::Draw() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            // AlignString("Press Enter to start game", screenWidth/2 ,screenHeight/2, 20, darkGreen);
        } break;
        case STAGE: {
            spawn_snake.Draw();
            spawn_food.Draw();
            std::string score = std::to_string(spawn_snake.body.size());
            DrawText(score.c_str(), 10,10, 20, darkGreen);
        } break;
        case GAMEOVER: {
            std::string score = std::to_string(spawn_snake.body.size());
            DrawText(score.c_str(), screenWidth/2 ,screenHeight/2, 50, darkGreen);
            DrawText("Press Enter to restart game", screenWidth/2 + 20,screenHeight/2 + 20, 20, darkGreen);

        } break;
        default:
            break;
    }
}
void game::Update() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            if (IsKeyPressed(KEY_ENTER))
            {
                state.currentScreen = STAGE;
            }
        } break;
        case STAGE: {
            
        } break;
        case GAMEOVER: {
            if (IsKeyPressed(KEY_ENTER))
            {
                state.currentScreen = STAGE;
                InitGameObject();
            }
        } break;
        default: break;
    }
    if (FixUpdate(fix_update_time)) {
        spawn_snake.Update(state, spawn_food);
        spawn_food.Update();
    }
}

bool game::FixUpdate(float interval) {
    double current_get_time = GetTime();
    if (current_get_time - last_get_time >= interval) {
        last_get_time = current_get_time;
        return true;
    } else {
        return false;
    }
}