#include "include/snake.h"

Snake::Snake()
{
    body.reserve(300);
    // Reset();
}

Snake::~Snake() {
}

void Snake::Reset(Room& room) {
    // setting snake init move direction by checking entrance position
    GridPosition spawn_pos = room.GetEntrance();
    DoorSide door_side = room.GetDoorSide(spawn_pos);
    if (door_side == DoorSide::LEFT) { 
        snake_move = dir_right;
        queued_direction = dir_right;
        spawn_pos.x += 1;
    } else if (door_side == DoorSide::RIGHT) { 
        snake_move = dir_left;
        queued_direction = dir_left;
        spawn_pos.x -= 1;
    } else if (door_side == DoorSide::UP) { 
        snake_move = dir_down;
        queued_direction = dir_down;
        spawn_pos.y += 1;
    } else if (door_side == DoorSide::DOWN) { 
        snake_move = dir_up;
        queued_direction = dir_up;
        spawn_pos.y -= 1;
    }
    TraceLog(LOG_ERROR, "snake spawn direction: %d", door_side);
    has_queued_direction = false;
    movement_elapsed = 0.0f;
    animation_elapsed = 0.0f;

    frame_counter = 0;
    flip_frame = 1;

    body.clear();

    unfold_steps = 0;
    
    for (int i=0; i< init_length; i++) {
        body.push_back(snake_body{spawn_pos, snake_move, snake_move, false});
    }
}

void Snake::EnterRoom(Room& room) {
    // setting snake init move direction by checking entrance position
    GridPosition spawn_pos = room.GetEntrance();
    DoorSide door_side = room.GetDoorSide(spawn_pos);
    if (door_side == DoorSide::LEFT) { 
        snake_move = dir_right;
        queued_direction = dir_right;
        spawn_pos.x += 1;
    } else if (door_side == DoorSide::RIGHT) { 
        snake_move = dir_left;
        queued_direction = dir_left;
        spawn_pos.x -= 1;
    } else if (door_side == DoorSide::UP) { 
        snake_move = dir_down;
        queued_direction = dir_down;
        spawn_pos.y += 1;
    } else if (door_side == DoorSide::DOWN) { 
        snake_move = dir_up;
        queued_direction = dir_up;
        spawn_pos.y -= 1;
    }
    has_queued_direction = false;
    movement_elapsed = 0.0f;
    animation_elapsed = 0.0f;

    frame_counter = 0;
    flip_frame = 1;

    unfold_steps = 0;
    
    for (int i=0; i< init_length; i++) {
        body.push_back(snake_body{spawn_pos, snake_move, snake_move, false});
    }
}

void Snake::DrawSnakePart(const Texture2D& texture,Rectangle draw_sprite, Rectangle draw_pos,direction dir) {
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
        texture,
        draw_sprite,
        draw_pos,
        origin,
        rotation,
        WHITE
    );
}

void Snake::Draw(const GameTextures& textures) {

    // for (unsigned int i=0; i<body.size(); i++) {
    for (int i=body.size()-1; i >= 0; i--) {
        // adding cell/2 due to define origin (cell/2, cell/2), function will re calculate the dest by decrease it position by cell/2, so we adding the cell/2 to offset that
        Rectangle draw_pos = 
        {   
            static_cast<float> (body[i].position.x * cellsize + cellsize/2.0f), 
            static_cast<float> (body[i].position.y * cellsize + cellsize/2.0f),
            static_cast<float> (cellsize),
            static_cast<float> (cellsize)
        };
        if (i==0) {
            Rectangle head_sprite = textures.GetSpriteRect(SpriteID::SNAKE_HEAD);
            head_sprite.height *= flip_frame;
            DrawSnakePart(textures.GetTexture(), head_sprite, draw_pos, body[i].cur_dir);
        }
        // draw tail
        else if (i==body.size()-1) {
            Rectangle tail_sprite = textures.GetSpriteRect(SpriteID::SNAKE_TAIL);
            tail_sprite.height *= flip_frame;
            DrawSnakePart(textures.GetTexture(),tail_sprite, draw_pos, body[i].cur_dir);
        }
        else if (body[i].is_turn) {
            Rectangle turn_sprite = textures.GetSpriteRect(SpriteID::SNAKE_TURN);
            if ((body[i].pre_dir == dir_right && body[i].cur_dir == dir_down) ||
                (body[i].pre_dir == dir_up && body[i].cur_dir == dir_left)) {
                    DrawSnakePart(textures.GetTexture(),turn_sprite, draw_pos, dir_right);
            } else if ((body[i].pre_dir == dir_right && body[i].cur_dir == dir_up) ||
                (body[i].pre_dir == dir_down && body[i].cur_dir == dir_left)) {
                    DrawSnakePart(textures.GetTexture(),turn_sprite, draw_pos, dir_down);
            } else if ((body[i].pre_dir == dir_down && body[i].cur_dir == dir_right) ||
                (body[i].pre_dir == dir_left && body[i].cur_dir == dir_up)) {
                    DrawSnakePart(textures.GetTexture(),turn_sprite, draw_pos, dir_left);
            } else if ((body[i].pre_dir == dir_left && body[i].cur_dir == dir_down) ||
                (body[i].pre_dir == dir_up && body[i].cur_dir == dir_right)) {
                    DrawSnakePart(textures.GetTexture(),turn_sprite, draw_pos, dir_up);
                }
        }
        // draw body
        else {
            Rectangle body_sprite = textures.GetSpriteRect(SpriteID::SNAKE_BODY);
            body_sprite.height *= flip_frame;
            DrawSnakePart(textures.GetTexture(),body_sprite, draw_pos, body[i].cur_dir);
        }
        // draw turn body  
        // std::cout << "body turn[" << i<< "] " << body[i].is_turn << std::endl;
    }   
}

void Snake::TailCut(int cut_index) {
    if (cut_index <= 0 ||
        cut_index >= static_cast<int>(body.size()))
    {
        return;
    }

    body.resize(static_cast<std::size_t>(cut_index));
}

void Snake::ReadInput() {
    direction requested_direction;
    if (IsKeyPressed(KEY_RIGHT)){
        requested_direction = dir_right;
    }
    else if (IsKeyPressed(KEY_LEFT)) {
        requested_direction = dir_left;
    }
    else if (IsKeyPressed(KEY_UP)){
        requested_direction = dir_up;
    }
    else if (IsKeyPressed(KEY_DOWN)) {
        requested_direction = dir_down;
    } else {
        return;
    }
    bool current_is_horizontal = 
        snake_move == dir_left ||
        snake_move == dir_right;
    bool requested_is_horizontal =
        requested_direction == dir_left ||
        requested_direction == dir_right;
    if (current_is_horizontal == requested_is_horizontal)
    {
        return;
    }

    queued_direction = requested_direction;
    has_queued_direction = true;
}

void Snake::MoveSnake() {
    if (body.empty()) {
        return;
    }
    snake_body new_head = body.front();
    body.pop_back();
    body.insert(body.begin(), new_head);
    body.front().is_turn = false;
    if (IsUnfolding()) {
        unfold_steps++; // snake make a valid move, increase unfold_step
    }
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
// void Snake::CheckSnakeState() {
//     // check hit wall
//     if (body.front().position.x > cellcount_width ||
//         body.front().position.x < 0 ||
//         body.front().position.y > cellcount_height ||
//         body.front().position.y < 0
//     ) {
//         // gamestate.currentScreen = GAMEOVER;

//     }
//     // check hit itslef -> cut tail
//     for (unsigned int i=1; i < body.size(); i++) {
//         if (body.front().position == body[i].position) {
//             // std::cout << 'cut index:' << i << std::endl;
//             TailCut(i);
//             break;
//         }
//     }
// }

void Snake::FixedUpdateAnimation(float delta_time) {
    animation_elapsed += delta_time;

    if (animation_elapsed >= animation_interval)
    {
        animation_elapsed -= animation_interval;
        flip_frame *= -1;
    }
}

bool Snake::UpdateMovement(float delta_time) {
    movement_elapsed += delta_time;

    if (movement_elapsed < movement_interval) {
        return false;
    }

    movement_elapsed -= movement_interval;

    if (has_queued_direction) {
        snake_move = queued_direction;
        has_queued_direction = false;

        body.front().pre_dir = body.front().cur_dir;
        body.front().cur_dir = snake_move;
        body.front().is_turn = true;
    }

    MoveSnake();
    // CheckSnakeState();

    return true;
}

void Snake::SetMovementInterval(float interval) {
    if (interval > 0.0f)
    {
        movement_interval = interval;
    }
}

bool Snake::IsUnfolding() {
    return unfold_steps < static_cast<int>(body.size()) - 1;
}


bool Snake::IsInvicible() {
    return unfold_steps < 2;
}

GridPosition Snake::GetHeadPosition() const {
    return body.front().position;
}