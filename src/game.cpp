#include "include/game.h"

void game::InitGameObject() {
    state = gamestate();
    state.currentScreen = MAIN_MENU;
    spawn_snake = Snake();
    spawn_food = Food();
    spawn_food.SetFoodPosition(spawn_snake);
}
void game::Draw() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            DrawText("Press Enter to start game", screenWidth/2 ,screenHeight/2, 20, darkGreen);
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
            if (FixUpdate(fix_update_time)) {
                spawn_snake.Update();
                spawn_food.Update();
                if (SnakeCollision(spawn_snake, spawn_food)) {
                    spawn_food.OnSnakeEnter(spawn_snake);
                }
            }
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

bool game::FixUpdate(float interval) {
    double current_get_time = GetTime();
    if (current_get_time - last_get_time >= interval) {
        last_get_time = current_get_time;
        return true;
    } else {
        return false;
    }
}

bool game::SnakeCollision(Snake& snake, GameObject object) {
    if (snake.body.front().position == object.GetPosition()) {
        return true;
    } else {
        return false;
    }
}