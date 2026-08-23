#include "include/game.h"

game::game() {
    game_sprite = LoadTexture("sprite/game_sprite.png");
    SetTextureFilter(game_sprite, TEXTURE_FILTER_POINT);
    spawn_food.SetSpriteTexture(game_sprite);
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
    InitGround();
    wall_bricks.clear();
    wall_cells.assign(cellcount_width * cellcount_height, false);
    InitStationaryWall();
    spawn_food.Reset(FoodType::Apple);
    spawn_food.SetFoodPosition(spawn_snake, wall_cells);
    last_get_time = GetTime();
}

void game::Draw() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            DrawText("Press Enter to start game", screenWidth/2 ,screenHeight/2, 20, darkGreen);
        } break;
        case STAGE: {
            DrawGround();
            for (unsigned int i=0; i<wall_bricks.size(); i++) {
                wall_bricks[i].Draw();
            }
            spawn_snake.Draw();
            spawn_food.Draw();
            DrawScorePopup();
            DrawUI();
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
            UpdateScorePopup();
            if (spawn_snake.body.empty()) {
                state.currentScreen = GAMEOVER;
                break;
            }
            spawn_snake.SetSpeedBoost(IsKeyDown(KEY_X));
            spawn_snake.ReadInput();
            if (FixUpdate(spawn_snake.move_interval)) {
                int removed_body_count = spawn_snake.Update();
                if (removed_body_count > 0) {
                    float requested_penalty = removed_body_count * 2.0f;
                    float deducted_score = requested_penalty < state.score ? requested_penalty : state.score;
                    state.score -= deducted_score;
                    if (deducted_score > 0.0f) {
                        ShowScorePopup(spawn_snake.body.front().position, -deducted_score);
                    }
                }
                if (IsWallCell(spawn_snake.body.front().position)) {
                    state.currentScreen = GAMEOVER;
                    break;
                }
                spawn_food.Update();
                if (SnakeCollision(spawn_snake, spawn_food)) {
                    Vector2 eaten_position = spawn_food.GetPosition();
                    int food_score = spawn_food.GetScore();
                    UpdateComboCounter(food_score, spawn_food.max_score);
                    float gained_score = GetFoodScoreWithCombo(food_score);
                    state.score += gained_score;
                    ShowScorePopup(eaten_position, gained_score);
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
    return cell_index < 0 || static_cast<size_t>(cell_index) >= wall_cells.size() || wall_cells[cell_index];
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

void game::DrawUI() {
    int ui_y = cellcount_height * cellsize;
    Color ui_background = {8, 18, 27, 255};
    DrawRectangle(0, ui_y, screenWidth, screenHeight - ui_y, ui_background);
    DrawLineEx(Vector2{0.0f, static_cast<float>(ui_y)},
               Vector2{static_cast<float>(screenWidth), static_cast<float>(ui_y)},
               2.0f, darkGreen);

    int text_y = ui_y + 30;
    DrawText(TextFormat("Score: %.1f", state.score), 16, text_y, 20, darkGreen);
    DrawText(TextFormat("Combo: %d", combo_counter), 220, text_y, 20, darkGreen);
    DrawText(TextFormat("Multiplier: %.1fx", score_multiplier), 400, text_y, 20, darkGreen);
}

void game::ShowScorePopup(Vector2 position, float value) {
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
    const char* popup_text = TextFormat("%+.1f", score_popup_value);
    int font_size = 16;
    int text_width = MeasureText(popup_text, font_size);
    float draw_x = score_popup_position.x * cellsize + cellsize / 2.0f - text_width / 2.0f;
    float draw_y = score_popup_position.y * cellsize - animation_progress * 20.0f;
    if (draw_y < 2.0f) {
        draw_y = 2.0f;
    }

    Color popup_color = score_popup_value < 0.0f ? Color{218, 74, 66, 255} : darkGreen;
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

float game::GetFoodScoreWithCombo(int food_score) {
    return food_score * score_multiplier;
}
