#include "test.h"

void TestApp::ModifySnakeHp(Snake& snake, int index) {
    if (index <0 || static_cast<std::size_t>(index) >= snake.body.size()) {
        return;
    }
    if (IsKeyPressed(KEY_A)){
        snake.DamageNode(snake.body[index].position, 1);
    }
}