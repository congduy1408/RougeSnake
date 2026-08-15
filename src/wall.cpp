#include "include/wall.h"
#include "include/snake.h"

void Brick::SetBrickPos(Vector2 pos) {
    SetPosition(pos);
}

void Brick::Draw() {
    DrawRectangle(position.x * cellsize, position.y * cellsize, cellsize, cellsize, darkGreen);
}

void Brick::Update() {

}

void Brick::OnSnakeEnter(Snake& snake) {

}