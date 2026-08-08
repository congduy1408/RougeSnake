#include "include/food.h"

food::food() {

}

void food::SetSnake(std::vector<Vector2> _snake) {
    checksnake = _snake;
}

bool food::CollideSnakePosition() {
    for (unsigned int i=0; i< checksnake.size(); i++) {
        if (Vector2Equals(position, checksnake[i])) {
            return true;
        }
    }
    return false;
} 

void food::SetFoodPosition() {
    position = RandomPosition();
    while(CollideSnakePosition()) {
        position = RandomPosition();
    }
}

void food::Draw() {
    DrawRectangle(position.x * cellsize, position.y * cellsize, cellsize, cellsize, darkGreen);
}

void food::Update() {}
