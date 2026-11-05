#include "test.h"

void TestApp::RecruitSoldier(Snake& snake) {
    if (IsKeyPressed(KEY_B)) snake.Grow(SoldierType::Basic);
    else if (IsKeyPressed(KEY_V)) snake.Grow(SoldierType::Veteran);
    else if (IsKeyPressed(KEY_R)) snake.Grow(SoldierType::Archer);
}

void TestApp::ModifySnakeHp(Snake& snake, int index, std::vector<Soldier>& detached_soldiers) {
    if (index <0 || static_cast<std::size_t>(index) >= snake.body.size()) {
        return;
    }
    if (IsKeyPressed(KEY_A)){
        std::vector<Soldier> detached = snake.DamageNode(snake.body[index].position, 1);
        detached_soldiers.insert(detached_soldiers.end(), detached.begin(), detached.end());
    }
}
