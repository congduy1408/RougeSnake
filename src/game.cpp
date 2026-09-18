#include "include/game.h"

void game::InitGameObject() {
    
    state = gamestate();
    state.currentScreen = MAIN_MENU;
    combo_counter = 0;
    score_multiplier = 1;

    board.Reset();
    spawn_snake.Reset();
    spawn_food.ResetScore();

    spawn_food.SetFoodPosition(spawn_snake, board);
    last_get_time = GetTime();
}

void game::Draw() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            DrawText("Press Enter to start game", screenWidth/2 ,screenHeight/2, 20, darkGreen);
        } break;
        case STAGE: {
            board.Draw();
            spawn_snake.Draw();
            spawn_food.Draw();
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
    if (!snake_moved)
    {
        return;
    }
    //// move to game over
    if (board.IsBlocked(spawn_snake.body.front().position)) {
        state.currentScreen = GAMEOVER;
        return;
    }
    // check snake collision with food
    //// grow snake, get score, update combo, spawn new food
    //// reset combo if snake move out of food boundary
    if (SnakeCollision(spawn_snake, spawn_food)) {
        int food_score = spawn_food.GetScore();
        UpdateComboCounter(food_score, spawn_food.max_score);
        state.score += GetFoodScoreWithCombo(food_score);
        spawn_snake.Grow();
        spawn_food.SetFoodPosition(spawn_snake, board);

    }
    else if (spawn_food.UpdateBoundaryScore(
                 spawn_snake.body.front().position))
    {
        combo_counter = 0;
        score_multiplier = 1.0f;
    }
}

bool game::SnakeCollision(Snake& snake, GameObject object) const {
    if (snake.body.front().position == object.GetPosition()) {
        return true;
    } else {
        return false;
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

float game::GetFoodScoreWithCombo(int food_score) {
    return food_score * score_multiplier;
}
