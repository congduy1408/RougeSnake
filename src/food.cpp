#include "include/food.h"
#include "include/snake.h"

// void Food::SetSnake(snake snake) {
//     checksnake = _snake;
// }

bool Food::CollideSnakePosition(Snake& snake) {
    for (unsigned int i=0; i< snake.body.size(); i++) {
        if (Vector2Equals(position, snake.body[i].position)) {
            return true;
        }
    }
    return false;
} 

void Food::SetFoodPosition(Snake& snake) {
    position = RandomPosition();
    while(CollideSnakePosition(snake)) {
        position = RandomPosition();
    }
}

void Food::Draw() {
    DrawRectangle(position.x * cellsize, position.y * cellsize, cellsize, cellsize, darkGreen);
}

void Food::OnSnakeEnter(Snake& snake) {
    snake.Grow();
    SetFoodPosition(snake);
}

void Food::Update() {}

