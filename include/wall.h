#pragma once
#include "common.h"
#include "include/gameobject.h"

class Snake;

class Brick: public GameObject {
    public:
        Brick(Vector2 position, const Texture2D& sprite) :
        GameObject(position), wall_sprite(&sprite), sprite_direction(GetRandomDirection()) {};
        void SetBrickPos(Vector2 pos);
        void Draw(float sprite_x_offset = 0.0f);
        void Update();
        void OnSnakeEnter(Snake& snake) override;
    private:
        const Texture2D* wall_sprite = nullptr;
        direction sprite_direction = dir_right;
};
