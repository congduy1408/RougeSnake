#include "include/wall.h"
#include "include/snake.h"


void Brick::SetBrickPos(Vector2 pos) {
    SetPosition(pos);
}

void Brick::Draw() {
    if (wall_sprite == nullptr || wall_sprite->id == 0) {
        return;
    }

    Rectangle draw_sprite = Rectangle{0.0f, 16.0f, 16.0f, 16.0f};
    Rectangle draw_pos = 
    {   position.x * cellsize + cellsize/2, 
        position.y * cellsize + cellsize/2,
        16,
        16
    };
    Vector2 origin = {draw_pos.width/2, draw_pos.height/2};
    float rotation = 0;
    switch(sprite_direction) {
        case dir_down: 
            rotation = 90;
            break;
        case dir_up:
            rotation = -90;
            break;
        case dir_right:
            rotation = 0;
            break;
        case dir_left:
            rotation = 180;
            break;
        default:
            break;
    }
    // DrawRectangle(position.x * cellsize, position.y * cellsize, cellsize, cellsize, darkGreen);
    DrawTexturePro(
        *wall_sprite,
        draw_sprite,
        draw_pos,
        origin,
        rotation,
        WHITE
    );
}

void Brick::Update() {

}

void Brick::OnSnakeEnter(Snake& snake) {

}
