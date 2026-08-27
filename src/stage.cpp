#include "include/stage.h"

void StageProgress::Reset() {
    stage_index = 0;
    apple_count = 0;
    key_active = false;
    banner_remaining = banner_duration;
}

void StageProgress::Update(float delta_time) {
    if (banner_remaining <= 0.0f) {
        return;
    }

    banner_remaining -= delta_time;
    if (banner_remaining < 0.0f) {
        banner_remaining = 0.0f;
    }
}

bool StageProgress::RecordAppleEaten() {
    if (key_active) {
        return false;
    }

    apple_count++;
    if (apple_count >= GetFoodGoal()) {
        key_active = true;
    }
    return key_active;
}

void StageProgress::Advance() {
    stage_index++;
    apple_count = 0;
    key_active = false;
    banner_remaining = banner_duration;
}

int StageProgress::GetStageIndex() const {
    return stage_index;
}

int StageProgress::GetAppleCount() const {
    return apple_count;
}

int StageProgress::GetFoodGoal() const {
    return initial_food_goal + stage_index * food_goal_increase;
}

bool StageProgress::IsKeyActive() const {
    return key_active;
}

bool StageProgress::IsBannerVisible() const {
    return banner_remaining > 0.0f;
}

float StageProgress::GetBannerAlpha() const {
    if (banner_duration <= 0.0f) {
        return 0.0f;
    }

    float fade_duration = 0.5f;
    if (banner_remaining >= fade_duration) {
        return 1.0f;
    }
    return banner_remaining / fade_duration;
}
