#pragma once

#include "include/common.h"

enum class RockState {
    Warning,
    Falling,
    Settled,
    Expired
};

class FallingRock {
    public:
        FallingRock(Vector2 position, const Texture2D& sprite,
                    float warning_duration, float fall_duration, float settled_duration);
        bool Update(float delta_time);
        void Draw() const;
        Vector2 GetPosition() const;
        RockState GetState() const;
        bool IsSolid() const;
        bool IsExpired() const;

    private:
        Vector2 position = {};
        const Texture2D* sprite = nullptr;
        RockState state = RockState::Warning;
        float state_remaining = 0.0f;
        float warning_duration = 0.0f;
        float fall_duration = 0.0f;
        float settled_duration = 0.0f;

        void DrawShadow() const;
        void DrawRock(float vertical_offset) const;
};
