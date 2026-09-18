#include "include/wall.h"
#include "include/snake.h"

void Brick::SetBrickPos(GridPosition pos) {
    SetPosition(pos);
}

void Brick::Draw() const {
    DrawRectangle(position.x * cellsize, position.y * cellsize, cellsize, cellsize, darkGreen);
}

void Brick::Update() {

}

void Brick::OnSnakeEnter(Snake& snake) {

}