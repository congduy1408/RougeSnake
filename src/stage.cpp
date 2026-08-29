#include "include/stage.h"

#include <cstddef>

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

bool StageProgress::RecordAppleEaten(bool activate_key_at_goal) {
    if (key_active) {
        return false;
    }

    if (apple_count < GetFoodGoal()) {
        apple_count++;
    }
    if (activate_key_at_goal && apple_count >= GetFoodGoal()) {
        key_active = true;
    }
    return key_active;
}

void StageProgress::ActivateKey() {
    key_active = true;
}

void StageProgress::Advance() {
    stage_index++;
    apple_count = 0;
    key_active = false;
    banner_remaining = banner_duration;
}

void StageProgress::SetFoodGoalForStage(int stage_index, int food_goal) {
    if (stage_index < 0 || food_goal < 1) {
        return;
    }

    if (static_cast<std::size_t>(stage_index) >= food_goals_by_stage.size()) {
        food_goals_by_stage.resize(static_cast<std::size_t>(stage_index) + 1,
                                  default_food_goal);
    }
    food_goals_by_stage[stage_index] = food_goal;
}

int StageProgress::GetStageIndex() const {
    return stage_index;
}

int StageProgress::GetAppleCount() const {
    return apple_count;
}

int StageProgress::GetFoodGoal() const {
    if (stage_index >= 0 && static_cast<std::size_t>(stage_index) < food_goals_by_stage.size()) {
        return food_goals_by_stage[stage_index];
    }
    return default_food_goal;
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
