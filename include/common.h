#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <utility>
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>


const Color darkGreen = {20, 160, 133, 255};
extern int cellsize;
extern int cellcount_width;
extern int cellcount_height;
extern int screenWidth;
extern int screenHeight;
extern float fix_update_time;
// extern float sprite_update_time;

enum direction {
    dir_up,
    dir_down,
    dir_left,
    dir_right
};

enum class DoorSide {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    INVALID
};

struct GridPosition
{
    int x = 0;
    int y = 0;

    bool operator==(const GridPosition& other) const
    {
        return x == other.x &&
               y == other.y;
    }

    bool operator!=(const GridPosition& other) const
    {
        return !(*this == other);
    }
};

GridPosition RandomPosition(GridPosition random_range);
bool FixFrameUpdate(int fps, int &frame_counter);