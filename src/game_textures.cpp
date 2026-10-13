#include "include/game_textures.h"

bool GameTextures::LoadAll() {
    gametexture = LoadTexture("sprite/game_sprite.png");
    if (!IsTextureValid(gametexture)) {
        UnLoadAll();
        return false;
    }
    SetTextureFilter(gametexture, TEXTURE_FILTER_POINT);
    return true;
}
void GameTextures::UnLoadAll() {
    if (IsTextureValid(gametexture)) UnloadTexture(gametexture);
    gametexture = {};
}

const Texture2D& GameTextures::GetTexture() const{
    return gametexture;
}
Rectangle GameTextures::GetSpriteRect(SpriteID id) const{
    switch (id) {
        case SpriteID::SNAKE_HEAD:
            return {32, 0, 16, 16};
        case SpriteID::SNAKE_BODY:
            return {16, 0, 16, 16};
        case SpriteID::SNAKE_TAIL:
            return {0, 0, 16, 16};
        case SpriteID::SNAKE_TURN:
            return {48, 0, 16, 16};
        case SpriteID::WALL:
            return {0, 16, 16, 16};
        case SpriteID::GROUND:
            return {16, 16, 16, 16};
        case SpriteID::GAP:
            return {32, 16, 16, 16};
        case SpriteID::DOOR:
            return {0, 64, 16, 16};
        case SpriteID::ENTRANCE:
            return {0, 64, 16, 16};
        case SpriteID::FOOD:
            return {16, 32, 16, 16};
        default:
            return {0, 0, 0, 0};
    }
}