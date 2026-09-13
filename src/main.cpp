#include "include/common.h"
#include "include/snake.h"
#include "include/food.h"
#include "include/game.h"

#include <algorithm>

int main() 
{
    InitWindow(screenWidth, screenHeight, "Rouge Snake");
    SetTargetFPS(60);

    constexpr double fixed_time_step = 1.0 / 60.0;
    double accumulated_time = 0.0;

    {
        game game;
        game.InitGameObject();
        // std::cout << spawn_food.position << std::endl;
        while (!WindowShouldClose())
        {
            double frame_time = GetFrameTime();
            frame_time = std::min(frame_time, 0.25);
            accumulated_time += frame_time;
            
            game.ReadInput();
            while (accumulated_time >= fixed_time_step) {
                game.FixUpdate(static_cast<float>(fixed_time_step));
                accumulated_time -= fixed_time_step;
            }

            // Drawing
            BeginDrawing();
            ClearBackground(BLACK);
            game.Draw();
            EndDrawing();
        }
    }
    
    CloseWindow();
}

