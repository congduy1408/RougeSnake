#include "include/wall.h"
#include "include/snake.h"


Brick::Brick()
{
    wall_sprite = LoadTexture("sprite/game_sprite.png");
    SetTextureFilter(wall_sprite, TEXTURE_FILTER_POINT);
}

Brick::~Brick() {
    if (wall_sprite.id != 0) {
        UnloadTexture(wall_sprite);
    }
}

void Brick::SetBrickPos(Vector2 pos) {
    SetPosition(pos);
}

void Brick::Draw() {
    Rectangle draw_sprite = Rectangle{0.0f, 16.0f, 16.0f, 16.0f};
    Rectangle draw_pos = 
    {   body[i].position.x * cellsize + cellsize/2, 
        body[i].position.y * cellsize + cellsize/2,
        16,
        16
    };
    DrawRectangle(position.x * cellsize, position.y * cellsize, cellsize, cellsize, darkGreen);
}
void Snake::DrawSnakePart(Rectangle draw_sprite, Rectangle draw_pos,direction dir) {
    // DrawTextureRec(snake_sprite, head_sprite, Vector2{body[i].position.x * cellsize, body[i].position.y * cellsize}, WHITE);  // Draw part of the texture
    Vector2 origin = {draw_pos.width/2, draw_pos.height/2};
    float rotation = 0;
    switch(dir) {
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
    DrawTexturePro(
        snake_sprite,
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