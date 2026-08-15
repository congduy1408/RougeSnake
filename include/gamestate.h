#pragma once

enum GameScreen {MAIN_MENU, STAGE, GAMEOVER};
struct gamestate {
    GameScreen currentScreen = MAIN_MENU;
    float score = 0;
};
