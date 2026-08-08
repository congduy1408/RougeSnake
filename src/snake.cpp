#include "include/snake.h"

snake::snake()
{
    init_length = 3;
    snake_move = dir_right;
    body.reserve(300);
    body.push_back(Vector2{10,15});
    body.push_back(Vector2{9,15});
    body.push_back(Vector2{8,15});
}

void snake::Draw() {
    for (unsigned int i=0; i<body.size(); i++) {
        Rectangle bodypart = Rectangle{body[i].x * cellsize, body[i].y * cellsize, cellsize, cellsize};
        DrawRectangleRounded(bodypart, 0.1, 0.5, darkGreen);    
    }     
}

void snake::TailCut(int cut_index) {
    for (unsigned int i=cut_index; i < body.size(); i++) {
        body.pop_back();
    }
}

void snake::ReadInput() {
    if (IsKeyDown(KEY_RIGHT) && snake_move != dir_left){
        snake_move = dir_right;
    }
    else if (IsKeyDown(KEY_LEFT) && snake_move != dir_right) {
        snake_move = dir_left;
    }
    else if (IsKeyDown(KEY_UP) && snake_move != dir_down){
        snake_move = dir_up;
    }
    else if (IsKeyDown(KEY_DOWN) && snake_move != dir_up) {
        snake_move = dir_down;
    }
}

void snake::MoveSnake() {
    body.pop_back();
    body.insert(body.begin(), body.front());
    switch (snake_move) {
        case dir_up:
            body.front().y -= 1;
            break;
        case dir_down:
            body.front().y += 1;
            break;
        case dir_left:
            body.front().x -= 1;
            break;
        case dir_right:
            body.front().x += 1;
            break;
        default:
            break;
    }
}


void snake::CheckSnakeState(gamestate &gamestate, food &food) {
    if (Vector2Equals(body.front(), food.position)){
        food.SetFoodPosition();
        body.push_back(body.back());
    }
    // check hit wall
    if (body.front().x > cellcount_width ||
        body.front().x < 0 ||
        body.front().y > cellcount_height ||
        body.front().y < 0
    ) {
        gamestate.currentScreen = GAMEOVER;
    }
    // check hit itslef -> cut tail
    for (unsigned int i=1; i < body.size(); i++) {
        if (Vector2Equals(body.front(),body[i])) {
            // std::cout << 'cut index:' << i << std::endl;
            TailCut(i);
            break;
        }
    }
}
void snake::Update(gamestate &gamestate, food &food) {
    ReadInput();
    MoveSnake();
    CheckSnakeState(gamestate, food);
}