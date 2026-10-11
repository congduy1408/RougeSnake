#pragma once
#include "common.h"

enum class SpriteID {
    SNAKE_HEAD,
    SNAKE_BODY,
    SNAKE_TAIL,
    SNAKE_TURN,
    FOOD,
    GROUND,
    WALL,
    GAP,
    ENTRANCE,
    DOOR
};

class GameTextures {
    public:
        bool LoadAll();
        bool UnLoadAll();

        const Texture2D& GetTexture() const;
        Rectangle GetSpriteRect(SpriteID id) const;
    private:
        Texture2D gametexture;

};