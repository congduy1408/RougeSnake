#include "include/game.h"

void game::InitGameObject() {
    state = gamestate();
    state.currentScreen = MAIN_MENU;
    combo_counter = 0;
    score_multiplier = 1;
    spawn_snake = Snake();
    wall_bricks.clear();
    wall_cells.assign(cellcount_width * cellcount_height, false);
    InitStationaryWall();
    spawn_food = Food();
    spawn_food.SetFoodPosition(spawn_snake, wall_cells);
}

void game::Draw() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            DrawText("Press Enter to start game", screenWidth/2 ,screenHeight/2, 20, darkGreen);
        } break;
        case STAGE: {
            for (unsigned int i=0; i<wall_bricks.size(); i++) {
                wall_bricks[i].Draw();
            }
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
void game::Update() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            if (IsKeyPressed(KEY_ENTER)  || IsKeyPressed(KEY_KP_ENTER))
            {
                state.currentScreen = STAGE;
            }
        } break;
        case STAGE: {
            if (FixUpdate(fix_update_time)) {
                spawn_snake.Update();
                if (IsWallCell(spawn_snake.body.front().position)) {
                    state.currentScreen = GAMEOVER;
                    break;
                }
                spawn_food.Update();
                if (SnakeCollision(spawn_snake, spawn_food)) {
                    int food_score = spawn_food.GetScore();
                    UpdateComboCounter(food_score, spawn_food.max_score);
                    state.score += GetFoodScoreWithCombo(food_score);
                    spawn_snake.Grow();
                    spawn_food.SetFoodPosition(spawn_snake, wall_cells);
                } else {
                    if (spawn_food.UpdateBoundaryScore(spawn_snake.body.front().position)) {
                        combo_counter = 0;
                        score_multiplier = 1;
                    }
                }
            }
        } break;
        case GAMEOVER: {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
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

int game::CellIndex(Vector2 pos) {
    return (int)pos.y * cellcount_width + (int)pos.x;
}

bool game::IsWallCell(Vector2 pos) {
    int x = (int)pos.x;
    int y = (int)pos.y;
    if (x < 0 || x >= cellcount_width || y < 0 || y >= cellcount_height) {
        return true;
    }
    return wall_cells[CellIndex(pos)];
}

void game::AddWallBrick(Vector2 pos) {
    int x = (int)pos.x;
    int y = (int)pos.y;
    if (x < 0 || x >= cellcount_width || y < 0 || y >= cellcount_height) {
        return;
    }

    int cell_index = CellIndex(pos);
    if (wall_cells[cell_index]) {
        return;
    }

    wall_cells[cell_index] = true;
    wall_bricks.push_back(Brick(pos));
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

float game::GetFoodScoreWithCombo(int food_score) {
    return food_score * score_multiplier;
}
