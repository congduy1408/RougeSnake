#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <utility>
//#include <iostream>
#include <string>


const Color darkGreen = {20, 160, 133, 255};
extern int game_scale;
extern int cellsize;
extern int cellcount_width;
extern int cellcount_height;
extern int ui_cellcount_height;
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

Vector2 RandomPosition();
bool FixFrameUpdate(int fps, int &frame_counter);

direction GetRandomDirection();
