#include "include/food.h"
#include "include/snake.h"

// void Food::SetSnake(snake snake) {
//     checksnake = _snake;
// }

void Food::SetSpriteTexture(const Texture2D& sprite) {
    food_sprite = &sprite;
}

void Food::SetFoodType(FoodType new_type) {
    food_type = new_type;
}

FoodType Food::GetFoodType() const {
    return food_type;
}

void Food::Reset(FoodType new_type) {
    food_type = new_type;
    ResetScore();
}

Rectangle Food::GetSpriteSource() const {
    switch (food_type) {
        case FoodType::Key:
            return Rectangle{0.0f, 32.0f, 16.0f, 16.0f};
        case FoodType::Apple:
        default:
            return Rectangle{16.0f, 32.0f, 16.0f, 16.0f};
    }
}

bool Food::CollideSnakePosition(Snake& snake) {
    for (unsigned int i=0; i< snake.body.size(); i++) {
        if (Vector2Equals(position, snake.body[i].position)) {
            return true;
        }
    }
    return false;
} 

bool Food::CollideWallPosition(const std::vector<bool>& wall_cells) {
    int x = (int)position.x;
    int y = (int)position.y;
    if (x < 0 || x >= cellcount_width || y < 0 || y >= cellcount_height) {
        return true;
    }
    int cell_index = y * cellcount_width + x;
    return cell_index < 0 || static_cast<size_t>(cell_index) >= wall_cells.size() || wall_cells[cell_index];
}

bool Food::IsInsideBoundary(Vector2 pos) {
    int food_x = (int)position.x;
    int food_y = (int)position.y;
    int pos_x = (int)pos.x;
    int pos_y = (int)pos.y;

    return pos_x >= food_x - 1 && pos_x <= food_x + 1 &&
           pos_y >= food_y - 1 && pos_y <= food_y + 1;
}

bool Food::UpdateBoundaryScore(Vector2 snake_pos) {
    bool is_inside_boundary = IsInsideBoundary(snake_pos);
    if (snake_inside_boundary && !is_inside_boundary && score > 1) {
        score--;
        snake_inside_boundary = is_inside_boundary;
        return true;
    }
    snake_inside_boundary = is_inside_boundary;
    return false;
}

int Food::GetScore() {
    return score;
}

void Food::ResetScore() {
    score = max_score;
    snake_inside_boundary = false;
}

void Food::SetFoodPosition(Snake& snake) {
    position = RandomPosition();
    while(CollideSnakePosition(snake)) {
        position = RandomPosition();
    }
    ResetScore();
}

void Food::SetFoodPosition(Snake& snake, const std::vector<bool>& wall_cells) {
    position = RandomPosition();
    while(CollideSnakePosition(snake) || CollideWallPosition(wall_cells)) {
        position = RandomPosition();
    }
    ResetScore();
}

void Food::Draw() {
    DrawRectangleLines((position.x - 1) * cellsize, (position.y - 1) * cellsize, cellsize * 3, cellsize * 3, darkGreen);
    if (food_sprite != nullptr && food_sprite->id != 0) {
        Rectangle draw_pos = {
            position.x * cellsize + cellsize / 2.0f,
            position.y * cellsize + cellsize / 2.0f,
            static_cast<float>(cellsize),
            static_cast<float>(cellsize)
        };
        Vector2 origin = {draw_pos.width / 2.0f, draw_pos.height / 2.0f};
        DrawTexturePro(*food_sprite, GetSpriteSource(), draw_pos, origin, 0.0f, WHITE);
    }
    std::string score_text = std::to_string(score);
    DrawText(score_text.c_str(), position.x * cellsize, position.y * cellsize - cellsize, 14, darkGreen);
}

void Food::OnSnakeEnter(Snake& snake) {
    snake.Grow();
    SetFoodPosition(snake);
}

void Food::Update() {}
