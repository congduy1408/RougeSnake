#pragma once

enum GameScreen {MAIN_MENU, STAGE, GAMEOVER, PAUSE_MENU};
struct gamestate {
    GameScreen currentScreen = MAIN_MENU;
};
