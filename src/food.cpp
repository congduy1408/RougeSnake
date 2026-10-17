#include "include/food.h"
#include "include/snake.h"
#include "include/board.h"

// void Food::SetSnake(snake snake) {
//     checksnake = _snake;
// }

bool Food::CollideSnakePosition(Snake& snake) {
    for (unsigned int i=0; i< snake.body.size(); i++) {
        if (position == snake.body[i].position) {
            return true;
        }
    }
    return false;
} 

bool Food::CollideWallPosition(Board& board) {
    return !board.IsWalkable(position) || board.IsLethal(position) || board.IsDoor(position);
}

bool Food::IsInsideBoundary(GridPosition pos) {
    return pos.x >= position.x - 1 && pos.x <= position.x + 1 &&
           pos.y >= position.y - 1 && pos.y <= position.y + 1;
}

bool Food::UpdateBoundaryScore(GridPosition snake_pos) {
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

void Food::SetFoodPosition(Snake& snake, Board& board) {
    position = RandomPosition(GridPosition{board.GetWidth(), board.GetHeight()});
    int  search_limit = board.GetHeight() * board.GetWidth();
    while(CollideSnakePosition(snake) || CollideWallPosition(board)) {
        position = RandomPosition(GridPosition{board.GetWidth(), board.GetHeight()});
        search_limit--;
        if (search_limit == 0) {
            TraceLog(LOG_ERROR, "Failed to spawn food");
            break;
        }
    }
    ResetScore();
}

void Food::Draw(const GameTextures& textures) {
    DrawRectangleLines((position.x - 1) * cellsize, (position.y - 1) * cellsize, cellsize * 3, cellsize * 3, darkGreen);
    // DrawRectangle(position.x * cellsize, position.y * cellsize, cellsize, cellsize, darkGreen);
    
    std::string score_text = std::to_string(score);
    DrawText(score_text.c_str(), position.x * cellsize, position.y * cellsize - cellsize, 14, darkGreen);
    // draw food sprite
    Rectangle draw_rect = textures.GetSpriteRect(SpriteID::FOOD);
    Rectangle draw_dest = {
                static_cast<float> (position.x*cellsize), 
                static_cast<float> (position.y*cellsize),
                static_cast<float> (cellsize),
                static_cast<float> (cellsize)};
    Vector2 origin = {0, 0};
        float rotation = 0;
        DrawTexturePro(
            textures.GetTexture(),
            draw_rect,
            draw_dest,
            origin,
            rotation,
            WHITE
        );           
}

void Food::Update() {}
