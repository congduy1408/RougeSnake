#include "include/common.h"
#include "include/snake.h"
#include "include/food.h"
#include "include/game.h"


void AlignString(char *text, int fontSize, int posX, int posY, Color color) {
    // align the print point of text to center of string/ top left, top right
    int textLength = MeasureText(text, fontSize);
    std::cout << "text length" << textLength << std::endl;
    DrawText(text, posX + textLength/2, posY + fontSize/2, fontSize, color);
}

int main() 
{
    GameScreen currentScreen = MAIN_MENU;
    InitWindow(screenWidth, screenHeight, "My first RAYLIB program!");
    SetTargetFPS(60);

    game game;
    game.InitGameObject();
    // std::cout << spawn_food.position << std::endl;
    while (!WindowShouldClose())
    {
        game.Update();
        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);
        game.Draw();
        EndDrawing();
    }
    
    CloseWindow();
}

