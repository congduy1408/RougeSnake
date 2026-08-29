#include "include/snake.h"

Snake::Snake()
{
    body.reserve(300);
    turn_point_list.reserve(300);
    // snake_sprite = LoadTexture("sprite/snake.png");
    snake_sprite = LoadTexture("sprite/game_sprite.png");
    SetTextureFilter(snake_sprite, TEXTURE_FILTER_POINT);
    Reset();
}

Snake::~Snake() {
    if (snake_sprite.id != 0) {
        UnloadTexture(snake_sprite);
    }
}

void Snake::Reset(Vector2 head_position, direction start_direction, int start_length) {
    init_length = start_length < 3 ? 3 : start_length;
    move_interval = normal_move_interval;
    snake_move = start_direction;
    input_queue.clear();
    frame_counter = 0;
    flip_frame = 1;
    body.clear();
    turn_point_list.clear();
    for (int i = 0; i < init_length; i++) {
        Vector2 body_position = head_position;
        switch (start_direction) {
            case dir_up:
                body_position.y += i;
                break;
            case dir_down:
                body_position.y -= i;
                break;
            case dir_left:
                body_position.x += i;
                break;
            case dir_right:
                body_position.x -= i;
                break;
            default:
                break;
        }
        body.push_back(snake_body{body_position, start_direction, start_direction, false});
    }
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
        GetDrawTint()
    );
}

Color Snake::GetDrawTint() const {
    constexpr double blink_changes_per_second = 4.0;
    bool show_warning =
        body.size() == 3 &&
        static_cast<int>(GetTime() * blink_changes_per_second) % 2 == 0;

    // Keep some green and blue so colored sprite pixels remain visible under tinting.
    return show_warning ? Color{255, 70, 70, 255} : snake_tint;
}

void Snake::Draw() {
    if (FixFrameUpdate(fps, frame_counter)) {
        flip_frame = flip_frame * -1;
    }

    auto MakeMirroredSprite = [this](float x, float y) {
        return Rectangle{x, y, 16.0f, 16.0f * flip_frame};
    };

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
            //Rectangle head_sprite = Rectangle{32.0f, 0.0f, 16.0f, 16.0f};
            Rectangle head_sprite = MakeMirroredSprite(32.0f, 0.0f);
            DrawSnakePart(head_sprite, draw_pos, body[i].cur_dir);
        }
        // draw tail
        else if (i==body.size()-1) {
            Rectangle tail_sprite = MakeMirroredSprite(0.0f, 0.0f);
            DrawSnakePart(tail_sprite, draw_pos, body[i].cur_dir);
        }
        else if (body[i].is_turn) {
            Rectangle head_sprite = Rectangle{48.0f, 0.0f, 16.0f, 16.0f};
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
            Rectangle body_sprite = MakeMirroredSprite(16.0f, 0.0f);
            DrawSnakePart(body_sprite, draw_pos, body[i].cur_dir);
        }
        // draw turn body  
        // std::cout << "body turn[" << i<< "] " << body[i].is_turn << std::endl;
    }   
}

int Snake::TailCut(int cut_index) {
    if (body.empty()) {
        return 0;
    }

    size_t first_removed = cut_index > 0 ? static_cast<size_t>(cut_index) : 1;
    if (first_removed < body.size()) {
        int removed_count = static_cast<int>(body.size() - first_removed);
        body.resize(first_removed);
        return removed_count;
    }
    return 0;
}

void Snake::ReadInput() {
    if (body.empty()) {
        return;
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        QueueDirection(dir_right);
    }
    else if (IsKeyPressed(KEY_LEFT)) {
        QueueDirection(dir_left);
    }
    else if (IsKeyPressed(KEY_UP)) {
        QueueDirection(dir_up);
    }
    else if (IsKeyPressed(KEY_DOWN)) {
        QueueDirection(dir_down);
    }
}

bool Snake::QueueDirection(direction new_direction) {
    constexpr size_t max_queued_turns = 2;
    if (body.empty() || input_queue.size() >= max_queued_turns) {
        return false;
    }

    direction current_direction = input_queue.empty() ? snake_move : input_queue.back();
    bool same_axis =
        (new_direction == dir_left || new_direction == dir_right) ==
        (current_direction == dir_left || current_direction == dir_right);
    if (same_axis) {
        return false;
    }

    input_queue.push_back(new_direction);
    return true;
}

void Snake::MoveSnake() {
    if (body.empty()) {
        return;
    }

    if (!input_queue.empty()) {
        direction previous_direction = snake_move;
        snake_move = input_queue.front();
        input_queue.erase(input_queue.begin());

        // The old head cell becomes the corner body segment after this move.
        body.front().pre_dir = previous_direction;
        body.front().cur_dir = snake_move;
        body.front().is_turn = true;
        turn_point_list.push_back({body.front().position, snake_move, previous_direction, true});
    }

    snake_body next_head = body.front();
    body.pop_back();
    body.insert(body.begin(), next_head);
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
    if (body.empty()) {
        return;
    }
    body.push_back(body.back());
}

bool Snake::IsAlive() const {
    return body.size() >= 3;
}

void Snake::SetSpeedBoost(bool active, float multiplier) {
    if (multiplier <= 1.0f) {
        move_interval = normal_move_interval;
        return;
    }

    move_interval = active ? normal_move_interval / multiplier : normal_move_interval;
}

void Snake::SetTint(Color tint) {
    snake_tint = tint;
}

// snakestate snake::CheckSnakeState(gamestate &gamestate, food &food) {
int Snake::CheckSnakeState() {
    if (body.empty()) {
        return 0;
    }

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
            return TailCut(i);
        }
    }
    return 0;
}
int Snake::Update() {
    MoveSnake();
    return CheckSnakeState();
}
