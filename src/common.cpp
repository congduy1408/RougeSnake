#include "include/common.h"

int cellsize = 16;
int cellcount_width = 40;
int cellcount_height = 30;
int screenWidth = cellsize * cellcount_width;
int screenHeight = cellsize * cellcount_height;
float fix_update_time = 0.05;
// float sprite_update_time = 0.05;

GridPosition RandomPosition(GridPosition random_range) {
    int x = GetRandomValue(0, random_range.x-1);
    int y = GetRandomValue(0, random_range.y - 1);
    return GridPosition{x, y};
}

bool FixFrameUpdate(int fps, int &frame_counter) {
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
