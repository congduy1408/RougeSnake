#pragma once

#include "include/common.h"

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
        bool Update(float delta_time);
        void Draw() const;
        Vector2 GetPosition() const;
        RockState GetState() const;
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
        bool shake_started = false;

        void DrawShadow() const;
        void DrawRock(float vertical_offset) const;
};
