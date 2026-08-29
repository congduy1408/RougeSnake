#pragma once

class StageProgress {
    public:
        int initial_food_goal = 10;
        int food_goal_increase = 5;
        float banner_duration = 3.0f;

        void Reset();
        void Update(float delta_time);
        bool RecordAppleEaten(bool activate_key_at_goal = true);
        void ActivateKey();
        void Advance();
        int GetStageIndex() const;
        int GetAppleCount() const;
        int GetFoodGoal() const;
        bool IsKeyActive() const;
        bool IsBannerVisible() const;
        float GetBannerAlpha() const;

    private:
        int stage_index = 0;
        int apple_count = 0;
        bool key_active = false;
        float banner_remaining = 0.0f;
};
