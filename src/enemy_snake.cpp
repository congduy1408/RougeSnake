#include "include/enemy_snake.h"

void EnemySnake::Reset(Vector2 head_position, direction start_direction, int start_length) {
    snake.Reset(head_position, start_direction, start_length);
    snake.SetTint(Color{255, 120, 90, 255});
    snake.SetSpeedBoost(false);
    alive = true;
    boost_moves_remaining = 0;
    last_death_position = head_position;
    last_death_length = start_length;
    last_move_time = GetTime();
}

bool EnemySnake::Update(const std::vector<bool>& wall_cells) {
    if (!alive || !IsMoveReady()) {
        return false;
    }

    UpdateDirection(wall_cells);
    UpdateSpeedBoost();
    snake.Update();
    if (!snake.IsAlive() || IsWallCell(snake.body.front().position, wall_cells)) {
        Kill();
    }
    return true;
}

void EnemySnake::Draw() {
    if (alive) {
        snake.Draw();
    }
}

void EnemySnake::SetTarget(Vector2, bool) {
}

void EnemySnake::Kill() {
    if (!snake.body.empty()) {
        last_death_position = snake.body.front().position;
        last_death_length = static_cast<int>(snake.body.size());
    }
    alive = false;
    snake.body.clear();
}

bool EnemySnake::IsAlive() const {
    return alive && snake.IsAlive();
}

Snake& EnemySnake::GetSnake() {
    return snake;
}

const Snake& EnemySnake::GetSnake() const {
    return snake;
}

Vector2 EnemySnake::GetLastDeathPosition() const {
    return last_death_position;
}

int EnemySnake::GetLastDeathLength() const {
    return last_death_length;
}

void EnemySnake::ConfigureMovementSpeed(float new_base_multiplier,
                                        int new_boost_chance_percent,
                                        float new_boost_multiplier,
                                        int new_boost_min_moves,
                                        int new_boost_max_moves) {
    base_speed_multiplier = new_base_multiplier > 0.0f ? new_base_multiplier : 1.0f;
    speed_boost_chance_percent = new_boost_chance_percent;
    if (speed_boost_chance_percent < 0) {
        speed_boost_chance_percent = 0;
    } else if (speed_boost_chance_percent > 100) {
        speed_boost_chance_percent = 100;
    }
    speed_boost_multiplier = new_boost_multiplier > 1.0f ? new_boost_multiplier : 1.0f;
    speed_boost_min_moves = new_boost_min_moves > 0 ? new_boost_min_moves : 1;
    speed_boost_max_moves = new_boost_max_moves > 0 ? new_boost_max_moves : 1;
    if (speed_boost_max_moves < speed_boost_min_moves) {
        int temporary = speed_boost_min_moves;
        speed_boost_min_moves = speed_boost_max_moves;
        speed_boost_max_moves = temporary;
    }
    boost_moves_remaining = 0;
    snake.SetMovementSpeedMultiplier(base_speed_multiplier);
}

bool EnemySnake::IsMoveReady() {
    double current_time = GetTime();
    if (current_time - last_move_time < snake.move_interval) {
        return false;
    }

    last_move_time = current_time;
    return true;
}

void EnemySnake::UpdateDirection(const std::vector<bool>& wall_cells) {
    direction turn_options[2];
    if (snake.snake_move == dir_up || snake.snake_move == dir_down) {
        turn_options[0] = dir_left;
        turn_options[1] = dir_right;
    } else {
        turn_options[0] = dir_up;
        turn_options[1] = dir_down;
    }

    std::vector<direction> safe_turns;
    for (direction turn_direction : turn_options) {
        Vector2 next_position = GetNextPosition(snake.body.front().position, turn_direction);
        if (!IsWallCell(next_position, wall_cells)) {
            safe_turns.push_back(turn_direction);
        }
    }

    bool moving_into_wall = IsWallCell(
        GetNextPosition(snake.body.front().position, snake.snake_move), wall_cells);
    bool choose_random_turn = GetRandomValue(0, 4) == 0;
    if (safe_turns.empty() || (!moving_into_wall && !choose_random_turn)) {
        return;
    }

    int selected_turn = GetRandomValue(0, static_cast<int>(safe_turns.size()) - 1);
    snake.QueueDirection(safe_turns[selected_turn]);
}

void EnemySnake::UpdateSpeedBoost() {
    if (boost_moves_remaining > 0) {
        boost_moves_remaining--;
        if (boost_moves_remaining == 0) {
            snake.SetMovementSpeedMultiplier(base_speed_multiplier);
        }
        return;
    }

    if (speed_boost_chance_percent > 0 &&
        GetRandomValue(1, 100) <= speed_boost_chance_percent) {
        boost_moves_remaining = GetRandomValue(speed_boost_min_moves,
                                               speed_boost_max_moves);
        snake.SetMovementSpeedMultiplier(base_speed_multiplier * speed_boost_multiplier);
    }
}

bool EnemySnake::IsWallCell(Vector2 position, const std::vector<bool>& wall_cells) const {
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    if (x < 0 || x >= cellcount_width || y < 0 || y >= cellcount_height) {
        return true;
    }

    int cell_index = y * cellcount_width + x;
    return cell_index < 0 ||
           static_cast<size_t>(cell_index) >= wall_cells.size() ||
           wall_cells[cell_index];
}

Vector2 EnemySnake::GetNextPosition(Vector2 position, direction move_direction) const {
    switch (move_direction) {
        case dir_up:
            position.y -= 1;
            break;
        case dir_down:
            position.y += 1;
            break;
        case dir_left:
            position.x -= 1;
            break;
        case dir_right:
            position.x += 1;
            break;
        default:
            break;
    }
    return position;
}
