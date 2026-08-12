#include "include/common.h"
#include "include/snake.h"
#include "include/food.h"
#include "include/gamestate.h"

class game {
    public:
        gamestate state;
        Snake spawn_snake;
        Food spawn_food;
        double last_get_time;
    void InitGameObject();
    void Draw();
    void Update();
    bool FixUpdate(float interval);
    bool SnakeCollision(Snake& snake, GameObject object);
};