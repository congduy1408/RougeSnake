#pragma once

#include "include/common.h"

#include <cstddef>

enum class RockState {
    Shaking,
    Warning,
    Falling,
    Settled,
    Expired
};

class FallingRock {
    public:
        FallingRock(Vector2 position, const Texture2D& sprite,
                    float shake_duration, float warning_duration,
                    float fall_duration, float settled_duration);
        FallingRock(Vector2 position, const Texture2D& sprite, Rectangle sprite_source,
                    std::size_t item_definition_index, float shake_duration,
                    float warning_duration, float fall_duration);
        bool Update(float delta_time);
        void Draw() const;
        Vector2 GetPosition() const;
        RockState GetState() const;
        bool IsItemDrop() const;
        std::size_t GetItemDefinitionIndex() const;
        bool ShouldStartShake();
        bool IsSolid() const;
        bool IsExpired() const;

    private:
        Vector2 position = {};
        const Texture2D* sprite = nullptr;
        RockState state = RockState::Shaking;
        float state_remaining = 0.0f;
        float warning_duration = 0.0f;
        float fall_duration = 0.0f;
        float settled_duration = 0.0f;
        Rectangle sprite_source = {0.0f, 16.0f, 16.0f, 16.0f};
        std::size_t item_definition_index = static_cast<std::size_t>(-1);
        bool shake_started = false;

        void DrawShadow() const;
        void DrawRock(float vertical_offset) const;
};
