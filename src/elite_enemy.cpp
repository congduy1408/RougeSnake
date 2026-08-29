#include "include/elite_enemy.h"

#include <algorithm>
#include <queue>

void EliteEnemy::Reset(Vector2 head_position, direction start_direction, int start_length) {
    EnemySnake::Reset(head_position, start_direction, start_length);
    snake.SetTint(Color{95, 185, 255, 255});
    state = EliteEnemyState::DetectFood;
    target_available = false;
    random_turns_remaining = random_turns_per_target;
    route.clear();
    route_index = 0;
}

void EliteEnemy::SetTarget(Vector2 new_target_position, bool new_target_available) {
    bool target_changed =
        target_available != new_target_available ||
        (new_target_available && !Vector2Equals(target_position, new_target_position));
    target_position = new_target_position;
    target_available = new_target_available;
    if (target_changed) {
        state = EliteEnemyState::DetectFood;
        random_turns_remaining = random_turns_per_target;
        route.clear();
        route_index = 0;
    }
}

EliteEnemyState EliteEnemy::GetState() const {
    return state;
}

void EliteEnemy::UpdateDirection(const std::vector<bool>& wall_cells) {
    if (!target_available || snake.body.empty()) {
        EnemySnake::UpdateDirection(wall_cells);
        return;
    }

    if (state == EliteEnemyState::DetectFood || route_index >= route.size()) {
        if (!PlanRoute(wall_cells)) {
            EnemySnake::UpdateDirection(wall_cells);
            return;
        }
    }

    if (TryRandomTurn(wall_cells)) {
        state = EliteEnemyState::DetectFood;
        return;
    }

    direction planned_direction = route[route_index];
    Vector2 next_position = GetNextPosition(snake.body.front().position, planned_direction);
    if (IsWallCell(next_position, wall_cells) || IsOwnBodyCell(next_position)) {
        int failure_chance = obstacle_avoidance_failure_percent;
        if (failure_chance > 5) {
            failure_chance = 5;
        }
        if (failure_chance > 0 && GetRandomValue(1, 100) <= failure_chance) {
            return;
        }

        state = EliteEnemyState::DetectFood;
        if (!PlanRoute(wall_cells)) {
            EnemySnake::UpdateDirection(wall_cells);
            return;
        }
        planned_direction = route[route_index];
    }

    if (planned_direction != snake.snake_move) {
        snake.QueueDirection(planned_direction);
    }
    route_index++;
}

bool EliteEnemy::PlanRoute(const std::vector<bool>& wall_cells) {
    route.clear();
    route_index = 0;
    if (snake.body.empty()) {
        return false;
    }

    int start_x = static_cast<int>(snake.body.front().position.x);
    int start_y = static_cast<int>(snake.body.front().position.y);
    int target_x = static_cast<int>(target_position.x);
    int target_y = static_cast<int>(target_position.y);
    if (start_x < 0 || start_x >= cellcount_width || start_y < 0 ||
        start_y >= cellcount_height || target_x < 0 || target_x >= cellcount_width ||
        target_y < 0 || target_y >= cellcount_height) {
        return false;
    }

    int cell_count = cellcount_width * cellcount_height;
    int start_index = start_y * cellcount_width + start_x;
    int target_index = target_y * cellcount_width + target_x;
    std::vector<int> previous(cell_count, -1);
    std::vector<direction> arrival_direction(cell_count, dir_right);
    std::queue<int> open_cells;
    previous[start_index] = start_index;
    open_cells.push(start_index);

    const direction directions[4] = {dir_up, dir_down, dir_left, dir_right};
    while (!open_cells.empty() && previous[target_index] == -1) {
        int current_index = open_cells.front();
        open_cells.pop();
        Vector2 current_position = {
            static_cast<float>(current_index % cellcount_width),
            static_cast<float>(current_index / cellcount_width)
        };

        for (direction move_direction : directions) {
            if (current_index == start_index &&
                IsOppositeDirection(move_direction, snake.snake_move)) {
                continue;
            }
            Vector2 next_position = GetNextPosition(current_position, move_direction);
            int next_x = static_cast<int>(next_position.x);
            int next_y = static_cast<int>(next_position.y);
            if (next_x < 0 || next_x >= cellcount_width ||
                next_y < 0 || next_y >= cellcount_height) {
                continue;
            }
            int next_index = next_y * cellcount_width + next_x;
            bool is_target = next_index == target_index;
            if (previous[next_index] != -1 ||
                (!is_target && (IsWallCell(next_position, wall_cells) ||
                                IsOwnBodyCell(next_position)))) {
                continue;
            }
            previous[next_index] = current_index;
            arrival_direction[next_index] = move_direction;
            open_cells.push(next_index);
        }
    }

    if (previous[target_index] == -1) {
        return false;
    }
    for (int cell = target_index; cell != start_index; cell = previous[cell]) {
        route.push_back(arrival_direction[cell]);
    }
    std::reverse(route.begin(), route.end());
    state = EliteEnemyState::FollowRoute;
    return !route.empty();
}

bool EliteEnemy::TryRandomTurn(const std::vector<bool>& wall_cells) {
    if (random_turns_remaining <= 0 || random_turn_chance_percent <= 0 ||
        GetRandomValue(1, 100) > random_turn_chance_percent) {
        return false;
    }

    direction options[2];
    if (snake.snake_move == dir_up || snake.snake_move == dir_down) {
        options[0] = dir_left;
        options[1] = dir_right;
    } else {
        options[0] = dir_up;
        options[1] = dir_down;
    }

    std::vector<direction> safe_options;
    for (direction option : options) {
        Vector2 next_position = GetNextPosition(snake.body.front().position, option);
        if (!IsWallCell(next_position, wall_cells) && !IsOwnBodyCell(next_position)) {
            safe_options.push_back(option);
        }
    }
    if (safe_options.empty()) {
        return false;
    }

    int selected = GetRandomValue(0, static_cast<int>(safe_options.size()) - 1);
    snake.QueueDirection(safe_options[selected]);
    random_turns_remaining--;
    return true;
}

bool EliteEnemy::IsOwnBodyCell(Vector2 position) const {
    for (size_t i = 1; i < snake.body.size(); i++) {
        if (Vector2Equals(position, snake.body[i].position)) {
            return true;
        }
    }
    return false;
}

bool EliteEnemy::IsOppositeDirection(direction first, direction second) const {
    return (first == dir_up && second == dir_down) ||
           (first == dir_down && second == dir_up) ||
           (first == dir_left && second == dir_right) ||
           (first == dir_right && second == dir_left);
}
