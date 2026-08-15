#include "include/snake.h"

Snake::Snake()
{
    init_length = 3;
    snake_move = dir_right;
    body.reserve(300);
    turn_point_list.reserve(300);
    body.push_back(snake_body{Vector2{10,15}, dir_right, dir_right, false});
    body.push_back(snake_body{Vector2{9,15}, dir_right, dir_right, false});
    body.push_back(snake_body{Vector2{8,15}, dir_right, dir_right, false});
    snake_sprite = LoadTexture("sprite/snake.png");

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

void Snake::Draw() {
    for (unsigned int i=0; i<body.size(); i++) {
        // draw head
        // Rectangle bodypart = Rectangle{body[i].x * cellsize, body[i].y * cellsize, cellsize, cellsize};
        // DrawRectangleRounded(bodypart, 0.1, 0.5, darkGreen);

        // adding cell/2 due to define origin (cell/2, cell/2), function will re calculate the dest by decrease it position by cell/2, so we adding the cell/2 to offset that
        Rectangle draw_pos = 
        {   body[i].position.x * cellsize + cellsize/2, 
            body[i].position.y * cellsize + cellsize/2,
            16,
            16
        };
        if (i==0) {
            Rectangle head_sprite = Rectangle{32,0, 16,16};
            // DrawTextureRec(snake_sprite, head_sprite, Vector2{body[i].position.x * cellsize, body[i].position.y * cellsize}, WHITE);  // Draw part of the texture
            DrawSnakePart(head_sprite, draw_pos, body[i].cur_dir);
        }
        // draw tail
        else if (i==body.size()-1) {
            Rectangle head_sprite = Rectangle{0,0, 16,16};
            DrawSnakePart(head_sprite, draw_pos, body[i].cur_dir);
        } else if (body[i].is_turn) {
            Rectangle head_sprite = Rectangle{32,16, 16,16};
            if ((body[i].pre_dir == dir_right && body[i].cur_dir == dir_down) ||
                (body[i].pre_dir == dir_up && body[i].cur_dir == dir_left)) {
                    DrawSnakePart(head_sprite, draw_pos, dir_right);
            } else if ((body[i].pre_dir == dir_right && body[i].cur_dir == dir_up) ||
                (body[i].pre_dir == dir_down && body[i].cur_dir == dir_left)) {
                    DrawSnakePart(head_sprite, draw_pos, dir_down);
            } else if ((body[i].pre_dir == dir_down && body[i].cur_dir == dir_right) ||
                (body[i].pre_dir == dir_left && body[i].cur_dir == dir_up)) {
                    DrawSnakePart(head_sprite, draw_pos, dir_left);
            } else if ((body[i].pre_dir == dir_left && body[i].cur_dir == dir_down) ||
                (body[i].pre_dir == dir_up && body[i].cur_dir == dir_right)) {
                    DrawSnakePart(head_sprite, draw_pos, dir_up);
                }
        }
        // draw body
        else {
            Rectangle head_sprite = Rectangle{16,0, 16,16};
            DrawSnakePart(head_sprite, draw_pos, body[i].cur_dir);
        }
        // draw turn body  
        std::cout << "body turn[" << i<< "] " << body[i].is_turn << std::endl;
    }   
}

void Snake::TailCut(int cut_index) {
    for (unsigned int i=cut_index; i < body.size(); i++) {
        body.pop_back();
    }
}

void Snake::ReadInput() {
    bool snake_turn = false;
    if (IsKeyDown(KEY_RIGHT) && snake_move != dir_left && snake_move != dir_right){
        snake_move = dir_right;
        snake_turn = true;
    }
    else if (IsKeyDown(KEY_LEFT) && snake_move != dir_right && snake_move != dir_left) {
        snake_move = dir_left;
        snake_turn = true;
    }
    else if (IsKeyDown(KEY_UP) && snake_move != dir_down && snake_move != dir_up){
        snake_move = dir_up;
        snake_turn = true;
    }
    else if (IsKeyDown(KEY_DOWN) && snake_move != dir_up && snake_move != dir_down) {
        snake_move = dir_down;
        snake_turn = true;
    }
    // record the point snake change direction
    // the record will contain: change position + new direction
    if (snake_turn) {
        turn_point_list.push_back({body.front().position, snake_move});
        body.front().pre_dir = body.front().cur_dir;
        body.front().cur_dir = snake_move;
        body.front().is_turn = true;
    }
}

void Snake::MoveSnake() {
    body.pop_back();
    body.insert(body.begin(), body.front());
    body.front().is_turn = false;
    switch (snake_move) {
        case dir_up:
            body.front().position.y -= 1;
            break;
        case dir_down:
            body.front().position.y += 1;
            break;
        case dir_left:
            body.front().position.x -= 1;
            break;
        case dir_right:
            body.front().position.x += 1;
            break;
        default:
            break;
    }
    body.front().cur_dir = snake_move;
}

void Snake::Grow() {
    body.push_back(body.back());
}

// snakestate snake::CheckSnakeState(gamestate &gamestate, food &food) {
void Snake::CheckSnakeState() {
    // check hit wall
    if (body.front().position.x > cellcount_width ||
        body.front().position.x < 0 ||
        body.front().position.y > cellcount_height ||
        body.front().position.y < 0
    ) {
        // gamestate.currentScreen = GAMEOVER;

    }
    // check hit itslef -> cut tail
    for (unsigned int i=1; i < body.size(); i++) {
        if (Vector2Equals(body.front().position,body[i].position)) {
            // std::cout << 'cut index:' << i << std::endl;
            TailCut(i);
            break;
        }
    }
}
void Snake::Update() {
    ReadInput();
    MoveSnake();
    CheckSnakeState();
}