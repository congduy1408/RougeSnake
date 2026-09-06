#include "include/rock.h"

FallingRock::FallingRock(Vector2 start_position, const Texture2D& rock_sprite,
                         float shake_time, float warning_time,
                         float fall_time, float lifetime)
    : position(start_position),
      sprite(&rock_sprite),
      state_remaining(shake_time),
      warning_duration(warning_time),
      fall_duration(fall_time),
      settled_duration(lifetime) {
}

FallingRock::FallingRock(Vector2 start_position, const Texture2D& item_sprite,
                         Rectangle item_sprite_source, std::size_t definition_index,
                         float shake_time, float warning_time, float fall_time)
    : position(start_position),
      sprite(&item_sprite),
      state_remaining(shake_time),
      warning_duration(warning_time),
      fall_duration(fall_time),
      sprite_source(item_sprite_source),
      item_definition_index(definition_index) {
}

bool FallingRock::Update(float delta_time) {
    state_remaining -= delta_time;
    if (state_remaining > 0.0f) {
        return false;
    }

    switch (state) {
        case RockState::Shaking:
            state = RockState::Warning;
            state_remaining = warning_duration;
            break;
        case RockState::Warning:
            state = RockState::Falling;
            state_remaining = fall_duration;
            break;
        case RockState::Falling:
            state = RockState::Settled;
            state_remaining = settled_duration;
            return true;
        case RockState::Settled:
            state = RockState::Expired;
            state_remaining = 0.0f;
            break;
        case RockState::Expired:
            break;
    }
    return false;
}

void FallingRock::Draw(float sprite_x_offset) const {
    if (state == RockState::Warning) {
        DrawShadow();
        return;
    }
    if (state == RockState::Falling) {
        DrawShadow();
        float progress = fall_duration <= 0.0f ? 1.0f : 1.0f - state_remaining / fall_duration;
        DrawRock(-(1.0f - progress) * cellsize * 4.0f, sprite_x_offset);
        return;
    }
    if (state == RockState::Settled) {
        DrawRock(0.0f, sprite_x_offset);
    }
}

Vector2 FallingRock::GetPosition() const {
    return position;
}

RockState FallingRock::GetState() const {
    return state;
}

bool FallingRock::IsItemDrop() const {
    return item_definition_index != static_cast<std::size_t>(-1);
}

std::size_t FallingRock::GetItemDefinitionIndex() const {
    return item_definition_index;
}

bool FallingRock::ShouldStartShake() {
    if (shake_started || state != RockState::Shaking) {
        return false;
    }

    shake_started = true;
    return true;
}

bool FallingRock::IsSolid() const {
    return state == RockState::Settled && !IsItemDrop();
}

bool FallingRock::IsExpired() const {
    return state == RockState::Expired;
}

void FallingRock::DrawShadow() const {
    float pulse = 0.65f + 0.35f * static_cast<float>(GetTime() * 5.0 -
        static_cast<int>(GetTime() * 5.0));
    Color shadow = {125, 22, 22, static_cast<unsigned char>(150.0f + pulse * 80.0f)};
    int center_x = static_cast<int>(position.x * cellsize + cellsize / 2.0f);
    int center_y = static_cast<int>(position.y * cellsize + cellsize / 2.0f);
    DrawEllipse(center_x, center_y, cellsize * 0.48f, cellsize * 0.30f, shadow);
    DrawRectangleLines(static_cast<int>(position.x * cellsize),
                       static_cast<int>(position.y * cellsize),
                       cellsize, cellsize, Color{235, 70, 60, 255});
}

void FallingRock::DrawRock(float vertical_offset, float sprite_x_offset) const {
    Rectangle destination = {
        position.x * cellsize + cellsize / 2.0f,
        position.y * cellsize + cellsize / 2.0f + vertical_offset,
        static_cast<float>(cellsize),
        static_cast<float>(cellsize)
    };
    Vector2 origin = {destination.width / 2.0f, destination.height / 2.0f};
    if (sprite != nullptr && sprite->id != 0) {
        Color tint = IsItemDrop() ? WHITE : Color{170, 160, 150, 255};
        Rectangle draw_source = sprite_source;
        if (!IsItemDrop()) {
            draw_source.x += sprite_x_offset;
        }
        DrawTexturePro(*sprite, draw_source, destination, origin, 0.0f, tint);
        return;
    }

    DrawCircle(static_cast<int>(destination.x), static_cast<int>(destination.y),
               cellsize * 0.45f, Color{95, 91, 88, 255});
}
