#include "include/common.h"
#include "include/snake.h"
#include "include/food.h"
#include "include/game.h"
#include "include/audio_system.h"

#include <cstdlib>


void AlignString(char *text, int fontSize, int posX, int posY, Color color) {
    // align the print point of text to center of string/ top left, top right
    int textLength = MeasureText(text, fontSize);
    //std::cout << "text length" << textLength << std::endl;
    DrawText(text, posX + textLength/2, posY + fontSize/2, fontSize, color);
}

int main(int argc, char** argv)
{
    int debug_start_stage = 0;
    bool enable_music = true;
    bool enable_sound_effects = true;
    for (int i = 1; i < argc; i++) {
        const char* argument = argv[i];
        if (argument == nullptr) {
            continue;
        }

        if (TextIsEqual(argument, "--stage") && i + 1 < argc) {
            debug_start_stage = std::atoi(argv[++i]);
        } else if (TextIsEqual(argument, "--no-music")) {
            enable_music = false;
        } else if (TextIsEqual(argument, "--no-sfx")) {
            enable_sound_effects = false;
        } else if (TextIsEqual(argument, "--help")) {
            TraceLog(LOG_INFO,
                     "Usage: RougeSnake.exe [--stage N] [--no-music] [--no-sfx]");
            return 0;
        }
    }

    if (debug_start_stage < 0) {
        debug_start_stage = 0;
    }

    int requested_scale = game_scale > 0 ? game_scale : 1;
    InitWindow(screenWidth, screenHeight, "Rouge Snake");
    InitAudioDevice();

    int monitor = GetCurrentMonitor();
    int available_width = GetMonitorWidth(monitor);
    int available_height = GetMonitorHeight(monitor) - 80;
    int monitor_scale_x = available_width / screenWidth;
    int monitor_scale_y = available_height / screenHeight;
    int maximum_scale = monitor_scale_x < monitor_scale_y ? monitor_scale_x : monitor_scale_y;
    if (maximum_scale < 1) {
        maximum_scale = 1;
    }
    int render_scale = requested_scale < maximum_scale ? requested_scale : maximum_scale;
    int window_width = screenWidth * render_scale;
    int window_height = screenHeight * render_scale;
    Vector2 monitor_position = GetMonitorPosition(monitor);
    SetWindowSize(window_width, window_height);
    SetWindowPosition(
        static_cast<int>(monitor_position.x) + (available_width - window_width) / 2,
        static_cast<int>(monitor_position.y) + (GetMonitorHeight(monitor) - window_height) / 2
    );
    SetTargetFPS(60);
    RenderTexture2D game_target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(game_target.texture, TEXTURE_FILTER_POINT);

    AudioSystem audio;
    audio.Initialize();
    audio.SetMusicEnabled(enable_music);
    audio.SetSoundEffectsEnabled(enable_sound_effects);

    {
        game game(audio, debug_start_stage);
        game.InitGameObject();
        // std::cout << spawn_food.position << std::endl;
        while (!WindowShouldClose())
        {
            game.Update();

            BeginTextureMode(game_target);
            ClearBackground(BLACK);
            game.Draw();
            EndTextureMode();

            BeginDrawing();
            ClearBackground(BLACK);
            Rectangle source = {
                0.0f,
                0.0f,
                static_cast<float>(game_target.texture.width),
                -static_cast<float>(game_target.texture.height)
            };
            Rectangle destination = {
                0.0f,
                0.0f,
                static_cast<float>(GetScreenWidth()),
                static_cast<float>(GetScreenHeight())
            };
            DrawTexturePro(game_target.texture, source, destination, Vector2{0.0f, 0.0f},
                           0.0f, WHITE);
            EndDrawing();
        }
    }

    UnloadRenderTexture(game_target);
    audio.Shutdown();
    CloseAudioDevice();
    CloseWindow();
}

