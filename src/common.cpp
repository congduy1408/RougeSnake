#include "include/common.h"

// Use whole-number values to keep pixel-art edges sharp (1 = 640x480, 2 = 1280x960).
int game_scale = 3;
int cellsize = 16;
int cellcount_width = 40;
int cellcount_height = 25;
int ui_cellcount_height = 5;
int screenWidth = cellsize * cellcount_width;
int screenHeight = cellsize * (cellcount_height + ui_cellcount_height);
float fix_update_time = 0.05;
// float sprite_update_time = 0.05;

Vector2 RandomPosition() {
    float x = GetRandomValue(0, cellcount_width-1);
    float y = GetRandomValue(0, cellcount_height - 1);
    return Vector2{x, y};
}

bool FixFrameUpdate(int fps, int &frame_counter) {
    if (fps <= 0) {
        return false;
    }
    frame_counter++;
    if (frame_counter >= (60/fps)) {
        // change frame
        frame_counter = 0;
        return true;  
    }
    else {
        return false;
    }
}

direction GetRandomDirection()
{
    return static_cast<direction>(
        GetRandomValue(dir_up, dir_right)
    );
}
