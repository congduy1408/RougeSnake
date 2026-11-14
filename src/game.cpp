#include "include/game.h"
#include <sstream>

namespace {
const Color menu_background = {13, 20, 25, 255};
const Color menu_panel = {23, 34, 40, 255};
const Color menu_muted = {158, 178, 181, 255};
const Color menu_accent = {101, 225, 174, 255};

void DrawItemDescription(const std::string& text, int x, int y, int width) {
    std::istringstream words(text);
    std::string word, line;
    while (words >> word) {
        const std::string next = line.empty() ? word : line + " " + word;
        if (!line.empty() && MeasureText(next.c_str(), 14) > width) {
            DrawText(line.c_str(), x, y, 14, menu_muted);
            y += 21;
            line = word;
        } else line = next;
    }
    if (!line.empty()) DrawText(line.c_str(), x, y, 14, menu_muted);
}

Rectangle MenuButton() {
    return {screenWidth / 2.0f - 140, 326, 280, 48};
}

bool MenuButtonPressed() {
    return IsKeyPressed(KEY_ENTER) ||
        (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
         CheckCollisionPointRec(GetMousePosition(), MenuButton()));
}

void CenterMenuText(const char* text, int y, int size, Color color) {
    DrawText(text, (screenWidth - MeasureText(text, size)) / 2, y, size, color);
}

void DrawMenuScreen(bool defeated, bool restart_failed) {
    ClearBackground(menu_background);
    for (int x = 0; x < screenWidth; x += 32)
        DrawLine(x, 0, x, screenHeight, Fade(menu_muted, 0.05f));
    for (int y = 0; y < screenHeight; y += 32)
        DrawLine(0, y, screenWidth, y, Fade(menu_muted, 0.05f));

    DrawRectangle(40, 32, screenWidth - 80, screenHeight - 64, menu_panel);
    DrawRectangle(40, 32, 4, screenHeight - 64, defeated ? ORANGE : menu_accent);
    CenterMenuText("ROUGE SNAKE / SQUAD EXPEDITION", 55, 12, menu_muted);

    // The leader and three distinct soldier roles share one following path.
    const int start_x = screenWidth / 2 - 66;
    DrawLine(start_x + 10, 113, start_x + 130, 113, menu_muted);
    const Color roles[] = {menu_accent, SKYBLUE, ORANGE, VIOLET};
    for (int i = 0; i < 4; ++i) {
        DrawRectangle(start_x + i * 40, 103, 20, 20, roles[i]);
        DrawRectangleLines(start_x + i * 40 - 3, 100, 26, 26, Fade(roles[i], 0.4f));
    }

    CenterMenuText(defeated ? "LEADER DEFEATED" : "LEAD YOUR SQUAD", 151, 30,
                   defeated ? ORANGE : RAYWHITE);
    CenterMenuText(defeated ? "The expedition ends. A new squad awaits." :
                   "Recruit soldiers. Keep moving. Survive together.", 195, 16, menu_muted);

    if (defeated) {
        CenterMenuText("Start a fresh run with a new squad.", 247, 16, RAYWHITE);
        CenterMenuText("Your previous squad and room progress will reset.", 276, 14, menu_muted);
    } else {
        CenterMenuText("ARROW KEYS   Steer the leader", 238, 16, RAYWHITE);
        CenterMenuText("Touch waiting soldiers to recruit. Followers attack automatically.", 270, 12, menu_muted);
        CenterMenuText("Avoid walls and gaps. Protect the leader.", 292, 14, menu_muted);
    }

    const Rectangle button = MenuButton();
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), button);
    DrawRectangleRec(button, hovered ? menu_accent : Fade(menu_accent, 0.85f));
    CenterMenuText(defeated ? "RETRY EXPEDITION" : "START EXPEDITION", 342, 18, menu_background);
    CenterMenuText(restart_failed ? "Restart failed. Check room files and try again." :
                   "ENTER or click to begin   /   ESC to quit", 391, 12,
                   restart_failed ? ORANGE : menu_muted);
}
}


game::game(const RoomCatalog& catalog) : room_catalog(catalog) { }

void game::UseLeaderItem() {
    if (state.currentScreen != STAGE || spawn_snake.IsDead()) return;
    if (!inventory.UseSelected()) return;
    switch (inventory.items[inventory.selected].effect) {
        case ItemEffect::None:
        case ItemEffect::DawnVeil:
        case ItemEffect::GraveShade:
        case ItemEffect::HollowNight: // Passive; UseSelected rejects auras.
            break;
        case ItemEffect::TestSignal:
            item_feedback = "Test item activated";
            item_feedback_time = 1.0f;
            break;
        case ItemEffect::Fireball: {
            const GridPosition aim = LeaderAim();
            SoldierAction action;
            action.projectiles.push_back({spawn_snake.GetHeadPosition(), aim, 20.0f, 8, LeaderSpellDamage(),
                                          ProjectileVisual::Fireball});
            ProcessSoldierAttack(action);
            break;
        }
        case ItemEffect::SaintsHalo:
        case ItemEffect::IceSpike:
        case ItemEffect::LightningChain:
        case ItemEffect::WaterWall:
        case ItemEffect::EarthPillar:
        case ItemEffect::GraveBloom:
            if (!CastSpell(inventory.items[inventory.selected].effect)) {
                inventory.items[inventory.selected].cooldown_remaining = 0;
                item_feedback = "No valid target or placement";
                item_feedback_time = 1;
            }
            break;
    }
}

void game::SpawnTestItem(ItemCategory category) {
    GridPosition cell;
    if (!FindRecruitSpawnCell(cell)) return;
    const auto& fillers = GetFillerItems();
    const Item item = category == ItemCategory::Aura ? GetAuraItems()[test_aura_sequence++ % GetAuraItems().size()] :
                      category == ItemCategory::Spell ? GetSpellItems()[test_spell_sequence++ % GetSpellItems().size()] :
                      fillers[static_cast<std::size_t>(test_item_sequence) % fillers.size()];
    if (category == ItemCategory::Filler)
        test_item_sequence = (test_item_sequence + 1) % static_cast<int>(fillers.size());
    ground_items.push_back({item, cell, false});
}

void game::UpdateGroundItemCooldowns(float delta_time) {
    for (auto& pickup : ground_items)
        pickup.item.cooldown_remaining = std::max(0.0f, pickup.item.cooldown_remaining - delta_time);
}

void game::CollectCombatItem() {
    const GridPosition head = spawn_snake.GetHeadPosition();
    for (auto& pickup : ground_items) {
        if (pickup.position != head) pickup.waiting_for_departure = false;
    }
    for (auto it = ground_items.begin(); it != ground_items.end(); ++it) {
        if (it->position != head || it->waiting_for_departure) continue;
        if (inventory.items.size() < inventory.Capacity()) {
            // New pickups do not disturb the current selection unless it is empty.
            if (inventory.items.empty()) inventory.selected = 0;
            inventory.items.push_back(it->item);
            ground_items.erase(it);
        } else if (!inventory.items.empty()) {
            inventory.selected = std::min(inventory.selected, inventory.items.size() - 1);
            if (inventory.items[inventory.selected].category == ItemCategory::Aura) {
                inventory.items[inventory.selected] = it->item;
                ground_items.erase(it); // Discarded auras vanish; no collectible remnant.
            } else {
                std::swap(inventory.items[inventory.selected], it->item);
                it->waiting_for_departure = true;
            }
        }
        return; // At most one pickup per leader movement step.
    }
}

bool game::InitGameObject() {
    restart_failed = false;
    inventory = LeaderInventory{};
    ground_items.clear();
    menu_item = 0;
    item_feedback_time = 0.0f;
    item_feedback.clear();
    test_item_sequence = 0;
    test_spell_sequence = test_aura_sequence = 0;
    world_spells.clear();
    healing_shards = 0;
    healing_shard_active = false;
    selected_action = 0;
    menu_tab = 0;
    selected_soldier = 0;
    soldier_actions = false;
    confirm_detach = false;
    
    state = gamestate();
    state.currentScreen = MAIN_MENU;
    recruit_spawn_elapsed = 0.0f;
    detached_soldiers.clear();
    projectiles.clear();

    if (!room_template.LoadFromFile("map/testmap_down.json")) {
        TraceLog(LOG_ERROR, "Failed to load room template");
        return false;
    }
    TraceLog(LOG_INFO, "Loading room...");
    if (!room.Load(room_template, board)) {
        TraceLog(LOG_ERROR, "Failed to load room");
        return false;        
    }
    TraceLog(LOG_INFO, "Resetting snake...");
    spawn_snake.Reset(room);
    TraceLog(LOG_INFO, "Initialization completed");

    InitRoomObject();

    return true;
}

void game::InitRoomObject() {
    world_spells.clear();
    ground_items.clear();
    enemies.clear();
    for (const auto& marker : room.GetObjectMarkers()) {
        if (marker.name == "enemy") {
            TraceLog(LOG_INFO, "load enemy: %s", marker.name.c_str());
            // Create enemy at marker.position
            enemies.push_back(std::make_unique<Enemy>(marker.position));
        }
        else if (marker.name == "item") {
            // Create item at marker.position
        }
    }
    SpawnHealingShard();
}

void game::SpawnHealingShard() {
    // Reuse the bounded safe-ground scan after room enemies and squad are placed.
    healing_shard_active = false;
    healing_shard_active = FindRecruitSpawnCell(healing_shard_position);
}

void game::CollectHealingShard() {
    if (healing_shard_active && spawn_snake.GetHeadPosition() == healing_shard_position) {
        ++healing_shards;
        healing_shard_active = false;
    }
}

bool game::HealSoldier(std::size_t index) {
    if (index >= spawn_snake.body.size() || healing_shards <= 0) return false;
    Soldier& soldier = spawn_snake.body[index];
    if (soldier.hp <= 0 || soldier.hp >= soldier.stats.max_hp) return false;
    const int amount = inventory.HasAura(ItemEffect::DawnVeil) ? 2 : 1;
    soldier.hp += std::min(amount, soldier.stats.max_hp - soldier.hp);
    --healing_shards;
    return true;
}

bool game::Transition(DoorSide side) {
    // read random map from door side
    DoorSide opposite_side = GetOppositeSide(side);
    // load room template
    std::string room_path = room_catalog.SelectRandom(opposite_side);
    if (room_path.empty()) {
        TraceLog(LOG_ERROR, "Empty room list");
        return false;
    }
    RoomTemplate temp_template;
    if (!temp_template.LoadFromFile(room_path)) {
        TraceLog(LOG_ERROR, "Failed to load room template");
        return false;
    }
    // load room + board
    if (!room.Load(temp_template, board)) {
        TraceLog(LOG_ERROR, "Failed to load room");
        return false;        
    }
    room_template = std::move(temp_template);
    projectiles.clear();
    // Unrescued soldiers are left behind when the squad changes rooms.
    detached_soldiers.clear();
    // Move the attached squad into the new room.
    TraceLog(LOG_INFO, "Resetting snake..."); 
    spawn_snake.EnterRoom(room);

    InitRoomObject();

    return true;
}

DoorSide game::GetOppositeSide(DoorSide side) {
    switch (side) {
        case DoorSide::DOWN:
            return DoorSide::UP;
            break;
        case DoorSide::UP:
            return DoorSide::DOWN;
            break;
        case DoorSide::LEFT:
            return DoorSide::RIGHT;
            break;
        case DoorSide::RIGHT:
            return DoorSide::LEFT;
            break;
        default:
            return DoorSide::INVALID;
            break;
    }
}

void game::Draw(const GameTextures& textures) {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            DrawMenuScreen(false, false);
        } break;
        case PAUSE_MENU:
        case STAGE: {
            Camera2D room_camera = {};
            room_camera.offset = {0.0f, static_cast<float>(squad_hud_height)};
            room_camera.zoom = 1.0f;
            BeginMode2D(room_camera);
            board.Draw(textures);
            if (healing_shard_active) {
                const int x = healing_shard_position.x * cellsize;
                const int y = healing_shard_position.y * cellsize;
                DrawPoly({x + cellsize / 2.0f, y + cellsize / 2.0f}, 4, cellsize * 0.45f, 0, menu_accent);
                DrawRectangle(x + cellsize / 2 - 1, y + 4, 2, cellsize - 8, menu_background);
                DrawRectangle(x + 4, y + cellsize / 2 - 1, cellsize - 8, 2, menu_background);
            }
            for (const auto& pickup : ground_items) {
                const int x = pickup.position.x * cellsize;
                const int y = pickup.position.y * cellsize;
                DrawRectangle(x + 1, y + 1, cellsize - 2, cellsize - 2, GOLD);
                DrawText(TextFormat("%c", pickup.item.letter), x + 3, y + 2, 12, menu_background);
            }
            DrawSpellEffects();
            DrawDetachedSoldiers(textures);
            spawn_snake.Draw(textures);
            for (const auto& enemy : enemies) {
                enemy->Draw();
            }
            for (const auto& soldier : spawn_snake.body) {
                if (soldier.shield_charge) DrawCircleLines(soldier.position.x * cellsize + cellsize / 2,
                    soldier.position.y * cellsize + cellsize / 2, cellsize * 0.48f, GOLD);
            }
            DrawSoldierAttacks();
            for (const auto& projectile : projectiles) projectile.Draw();
            EndMode2D();
            DrawSquadHud();
            if (state.currentScreen == PAUSE_MENU) DrawPauseMenu();
        } break;
        case GAMEOVER: {
            DrawMenuScreen(true, restart_failed);
        } break;
        default:
            break;
    }
}

void game::DrawSquadHud() const {
    int basic = 0;
    int veteran = 0;
    int archer = 0;
    // The leader is body[0] and is not included in follower capacity or roles.
    for (std::size_t i = 1; i < spawn_snake.body.size(); ++i) {
        switch (spawn_snake.body[i].type) {
            case SoldierType::Basic: ++basic; break;
            case SoldierType::Veteran: ++veteran; break;
            case SoldierType::Archer: ++archer; break;
        }
    }

    DrawRectangle(0, 0, screenWidth, squad_hud_height, menu_panel);
    DrawText(TextFormat("SQUAD %d/%d",
        static_cast<int>(spawn_snake.GetFollowerCount()),
        static_cast<int>(spawn_snake.max_followers)), 16, 14, 18, menu_accent);
    DrawText(TextFormat("Basic x%d", basic), 180, 16, 16, SKYBLUE);
    DrawText(TextFormat("Veteran x%d", veteran), 320, 16, 16, ORANGE);
    DrawText(TextFormat("Archer x%d", archer), 480, 16, 16, VIOLET);

    DrawText("CIPHER", 12, 40, 12, menu_muted);
    const auto matches = inventory.CipherMatches();
    for (std::size_t i = 0; i < matches.size(); ++i) {
        if (i >= 8) break; // Full inventory/cipher details remain available in the menu.
        const int x = 72 + static_cast<int>(i) * 17;
        DrawText(TextFormat("%c", inventory.cipher[i]), x, 40, 12, matches[i] ? menu_accent : menu_muted);
    }
    DrawText(TextFormat("ITEMS %d/%d", static_cast<int>(inventory.items.size()),
             static_cast<int>(inventory.Capacity())), 222, 40, 12, menu_muted);
    const std::size_t first_item = inventory.selected >= 6 ? inventory.selected - 5 : 0;
    for (std::size_t i = first_item; i < inventory.items.size() && i < first_item + 6; ++i) {
        const int x = 322 + static_cast<int>(i - first_item) * 22;
        DrawRectangleLines(x, 35, 19, 22, i == inventory.selected ? menu_accent : menu_muted);
        DrawText(TextFormat("%c", inventory.items[i].letter), x + 5, 40, 12,
                 inventory.items[i].cooldown_remaining > 0 ? menu_muted : RAYWHITE);
    }
    DrawText(TextFormat("HEAL x%d", healing_shards), 492, 40, 16, menu_accent);
    if (inventory.selected < inventory.items.size()) {
        const auto& item = inventory.items[inventory.selected];
        if (item.category == ItemCategory::Aura)
            DrawText(TextFormat("A/D: Select  %s  -  Aura active while carried", item.name.c_str()), 12, 63, 10, menu_accent);
        else if (item.effect == ItemEffect::None)
            DrawText(TextFormat("A/D: Select  %s  -  Passive cipher item", item.name.c_str()), 12, 63, 10, menu_muted);
        else DrawText(TextFormat("A/D: Select  SPACE: Use  %s  %.1fs", item.name.c_str(), item.cooldown_remaining),
                      12, 63, 10, menu_muted);
    } else DrawText("A/D: Select   SPACE: Use   TAB: Menu", 12, 63, 10, menu_muted);
    if (item_feedback_time > 0) DrawText(item_feedback.c_str(), 400, 63, 10, menu_accent);
    DrawLine(0, squad_hud_height - 1, screenWidth, squad_hud_height - 1,
             Fade(menu_accent, 0.45f));
}

void game::DrawPauseMenu() const {
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));
    DrawRectangle(24, 36, screenWidth - 48, screenHeight - 72, menu_panel);
    DrawText("EXPEDITION MENU", 44, 54, 20, menu_accent);
    DrawText("PAUSED", screenWidth - 108, 58, 12, menu_muted);
    const char* tabs[] = {"SQUAD", "ITEMS", "ENEMIES", "BOSS TRAVEL"};
    for (int i = 0; i < 4; ++i) {
        const int x = 44 + i * 140;
        DrawText(tabs[i], x, 98, 14, menu_tab == i ? menu_accent : menu_muted);
        if (menu_tab == i) DrawRectangle(x, 120, 115, 2, menu_accent);
    }
    if (menu_tab == 0) {
        const std::size_t first = selected_soldier >= 8 ? selected_soldier - 7 : 0;
        for (std::size_t i = first; i < spawn_snake.body.size() && i < first + 8; ++i) {
            const Soldier& soldier = spawn_snake.body[i];
            const char* role = soldier.type == SoldierType::Archer ? "Archer" :
                               soldier.type == SoldierType::Veteran ? "Veteran" : "Basic";
            const int y = 150 + static_cast<int>(i - first) * 32;
            if (i == selected_soldier) DrawRectangle(40, y - 4, 204, 28, Fade(menu_accent, 0.16f));
            DrawText(i == 0 ? "Leader" : TextFormat("%d. %s", static_cast<int>(i), role),
                     52, y, 16, i == selected_soldier ? menu_accent : RAYWHITE);
        }
        if (selected_soldier < spawn_snake.body.size()) {
            const Soldier& soldier = spawn_snake.body[selected_soldier];
            DrawText(selected_soldier == 0 ? "LEADER" : "FOLLOWER", 274, 152, 18, RAYWHITE);
            DrawText(TextFormat("HP: %d / %d", soldier.hp, soldier.stats.max_hp), 274, 190, 18, menu_accent);
            const char* role = soldier.type == SoldierType::Archer ? "Archer" :
                               soldier.type == SoldierType::Veteran ? "Veteran" : "Basic";
            DrawText(selected_soldier == 0 ? "Leader details coming later." : role, 274, 222, 14, menu_muted);
            const bool can_heal = healing_shards > 0 && soldier.hp > 0 && soldier.hp < soldier.stats.max_hp;
            if (soldier_actions)
                DrawRectangle(264, selected_action == 0 ? 270 : 302, 326, 28, Fade(menu_accent, 0.12f));
            DrawText(selected_soldier == 0 ? "Detach unavailable for leader" : "Detach selected follower",
                     274, 276, 14, selected_soldier == 0 ? menu_muted : menu_accent);
            DrawText(TextFormat("Heal +%d HP (%d shards)", inventory.HasAura(ItemEffect::DawnVeil) ? 2 : 1, healing_shards), 274, 308, 14,
                     can_heal ? menu_accent : menu_muted);
            if (soldier_actions && selected_action == 1)
                DrawText(soldier.hp >= soldier.stats.max_hp ? "Already at full health." :
                         healing_shards == 0 ? "Collect a healing shard first." : "Uses one shard. Enter to heal.",
                         274, 402, 12, menu_muted);
            if (confirm_detach) {
                DrawText("Leave this soldier in the room?", 274, 354, 14, ORANGE);
                DrawText("Enter: Confirm   Esc: Cancel", 274, 380, 12, RAYWHITE);
            } else if (soldier_actions) {
                DrawText("Up/Down: Action   Enter: Use", 274, 354, 14, menu_muted);
            }
        }
    } else if (menu_tab == 1) {
        DrawText(TextFormat("LEADER ITEMS %d / %d", static_cast<int>(inventory.items.size()),
                 static_cast<int>(inventory.Capacity())), 44, 143, 16, RAYWHITE);
        const std::size_t first = menu_item >= 6 ? menu_item - 5 : 0;
        for (std::size_t i = first; i < inventory.items.size() && i < first + 6; ++i) {
            const int y = 178 + static_cast<int>(i - first) * 30;
            if (i == menu_item) DrawRectangle(40, y - 3, 220, 26, Fade(menu_accent, 0.16f));
            DrawText(TextFormat("%d. [%c] %s", static_cast<int>(i + 1), inventory.items[i].letter,
                     inventory.items[i].name.c_str()), 48, y, 12, i == inventory.selected ? menu_accent : RAYWHITE);
        }
        if (menu_item < inventory.items.size()) {
            const auto& item = inventory.items[menu_item];
            DrawText(item.name.c_str(), 284, 150, 16, RAYWHITE);
            DrawText(TextFormat("Letter: %c", item.letter), 284, 180, 18, menu_accent);
            if (item.category == ItemCategory::Aura)
                DrawText("Aura active - does not stack", 284, 214, 14, menu_accent);
            else if (item.effect == ItemEffect::None)
                DrawText("Passive cipher item", 284, 214, 14, menu_muted);
            else DrawText(TextFormat("Cooldown: %.1f / %.1f s", item.cooldown_remaining, item.cooldown),
                          284, 214, 14, menu_muted);
            DrawItemDescription(item.description, 284, 248, 300);
            if (item.id == "cryptbound_book")
                DrawText("Clue reveal: coming later.", 284, 328, 12, menu_muted);
            DrawText("Enter: Select item", 284, 356, 12, RAYWHITE);
        } else {
            DrawText("J: Filler   U: Cycle auras", 48, 185, 14, menu_muted);
            DrawText("P: Cycle spells", 48, 215, 14, menu_muted);
        }
        DrawText(TextFormat("CONSUMABLES   Healing Shards: %d", healing_shards), 44, 393, 14, menu_accent);
        DrawText("Use Heal in the Squad tab. Shards use no item slots.", 44, 422, 12, menu_muted);
    } else {
        const char* descriptions[] = {"", "Collected items and their effects.",
            "Information about encountered enemies.", "Travel to eligible boss rooms."};
        DrawText(descriptions[menu_tab], 52, 174, 18, RAYWHITE);
        DrawText("Coming later. No actions available yet.", 52, 216, 16, menu_muted);
    }
    DrawText("Q / W: Tabs     Up / Down: Select     Enter: Actions", 44, screenHeight - 92, 12, menu_muted);
    DrawText("Tab: Resume     Esc: Back / Resume", 44, screenHeight - 66, 12, menu_muted);
}

void game::ReadPauseMenuInput() {
    if (IsKeyPressed(KEY_TAB)) {
        state.currentScreen = STAGE;
        soldier_actions = confirm_detach = false;
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (confirm_detach) confirm_detach = false;
        else if (soldier_actions) soldier_actions = false;
        else state.currentScreen = STAGE;
        return;
    }
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_W)) {
        menu_tab = (menu_tab + (IsKeyPressed(KEY_Q) ? 3 : 1)) % 4;
        soldier_actions = confirm_detach = false;
        return;
    }
    if (menu_tab == 1) {
        if (inventory.items.empty()) return;
        menu_item = std::min(menu_item, inventory.items.size() - 1);
        if (IsKeyPressed(KEY_UP) && menu_item > 0) --menu_item;
        if (IsKeyPressed(KEY_DOWN) && menu_item + 1 < inventory.items.size()) ++menu_item;
        if (IsKeyPressed(KEY_ENTER)) inventory.selected = menu_item;
        return;
    }
    if (menu_tab != 0 || spawn_snake.body.empty()) return;
    selected_soldier = std::min(selected_soldier, spawn_snake.body.size() - 1);
    if (!soldier_actions) {
        if (IsKeyPressed(KEY_UP) && selected_soldier > 0) --selected_soldier;
        if (IsKeyPressed(KEY_DOWN) && selected_soldier + 1 < spawn_snake.body.size()) ++selected_soldier;
        if (IsKeyPressed(KEY_ENTER)) {
            soldier_actions = true;
            selected_action = selected_soldier == 0 ? 1 : 0;
        }
    } else {
        if (!confirm_detach && (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)))
            selected_action = 1 - selected_action;
        if (!IsKeyPressed(KEY_ENTER)) return;
        if (selected_action == 1) {
            HealSoldier(selected_soldier);
            return;
        }
        if (selected_soldier == 0) return;
        if (!confirm_detach) confirm_detach = true;
        else {
            Soldier detached;
            if (spawn_snake.DetachFollower(selected_soldier, detached))
                detached_soldiers.push_back(std::move(detached));
            selected_soldier = std::min(selected_soldier, spawn_snake.body.size() - 1);
            soldier_actions = confirm_detach = false;
        }
    }
}

void game::DrawDetachedSoldiers(const GameTextures& textures) const {
    Rectangle sprite = textures.GetSpriteRect(SpriteID::SNAKE_BODY);

    for (const auto& soldier : detached_soldiers) {
        if (soldier.ghost_time > 0.0f) {
            float elapsed = Soldier::initial_ghost_time - soldier.ghost_time;
            if (static_cast<int>(elapsed / 0.05f) % 2 != 0) continue;
        }
        Rectangle destination = {
            soldier.position.x * cellsize + cellsize / 2.0f,
            soldier.position.y * cellsize + cellsize / 2.0f,
            static_cast<float>(cellsize),
            static_cast<float>(cellsize)
        };

        float rotation = 0.0f;
        switch (soldier.cur_dir) {
            case dir_right: rotation = 0.0f; break;
            case dir_down:  rotation = 90.0f; break;
            case dir_left:  rotation = 180.0f; break;
            case dir_up:    rotation = -90.0f; break;
        }

        const Color tint = soldier.hp >= 3 ? WHITE : (soldier.hp == 2 ? BLUE : RED);
        DrawTexturePro(
            textures.GetTexture(),
            sprite,
            destination,
            {cellsize / 2.0f, cellsize / 2.0f},
            rotation,
            tint
        );
    }
}

void game::ReadInput() {
    switch (state.currentScreen) {
        case MAIN_MENU: {
            if (MenuButtonPressed())
            {
                state.currentScreen = STAGE;
            }
        } break;
        case STAGE: {
            if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_ESCAPE)) {
                state.currentScreen = PAUSE_MENU;
                selected_soldier = 0;
                soldier_actions = confirm_detach = false;
            } else {
                spawn_snake.ReadInput();
                if (IsKeyPressed(KEY_A)) inventory.Cycle(-1);
                if (IsKeyPressed(KEY_D)) inventory.Cycle(1);
                if (IsKeyPressed(KEY_SPACE) && !spawn_snake.IsDead()) UseLeaderItem();
                if (IsKeyPressed(KEY_J)) SpawnTestItem();
                if (IsKeyPressed(KEY_U)) SpawnTestItem(ItemCategory::Aura);
                if (IsKeyPressed(KEY_P)) SpawnTestItem(ItemCategory::Spell);
            }
        } break;
        case PAUSE_MENU: {
            ReadPauseMenuInput();
        } break;
        case GAMEOVER: {
            if (MenuButtonPressed())
            {
                if (InitGameObject()) {
                    state.currentScreen = STAGE;
                } else {
                    state.currentScreen = GAMEOVER;
                    restart_failed = true;
                    TraceLog(LOG_ERROR, "Restart failed: unable to initialize game");
                }
            }
        } break;
        default: break;
    }

}

void game::FixUpdate(float interval) {
    if (state.currentScreen != STAGE) {
        return;
    }
    if (spawn_snake.IsDead()) {
        state.currentScreen = GAMEOVER;
        return;
    }
    UpdateSpellTimers(interval);
    inventory.UpdateCooldowns(interval);
    UpdateGroundItemCooldowns(interval);
    item_feedback_time = std::max(0.0f, item_feedback_time - interval);
    // Update snake movement base on interval
    bool snake_moved = spawn_snake.UpdateMovement(interval);
    spawn_snake.ReduceGhostTime(interval);
    ReduceDetachedGhostTime(interval);
    spawn_snake.FixedUpdateAnimation(interval);
    // check snake collision with wall
    if (snake_moved)
    {
        GridPosition head_pos = spawn_snake.GetHeadPosition();
        //// move to game over
        if (!board.IsWalkable(head_pos)) {
            state.currentScreen = GAMEOVER;
            return;
        }
        if (board.IsLethal(head_pos)) {
            state.currentScreen = GAMEOVER;
            return;
        }
        // Check snake move to exit door
        if (board.IsDoor(head_pos)) {
            // check door side
            DoorSide exit_side = room.GetDoorSide(head_pos);
            // call change room function
            if (exit_side != DoorSide::INVALID) {
                TraceLog(LOG_INFO, "Snake reached exit door %d", exit_side);
                
                // adding transition logic
                Transition(exit_side);
                return; // stop process other gameobject for prepare next room
            }
        }
        HandleSoldierRescue();
        CollectHealingShard();
        CollectCombatItem();

    }
    // process enemy
    // collect enemy attack
    std::vector<EnemyAction> actions_list;
    Board enemy_board = board;
    for (int x = 0; x < board.GetWidth(); ++x)
        for (int y = 0; y < board.GetHeight(); ++y)
            if (SpellBlocksEnemy({x, y})) enemy_board.SetCell({x, y}, WALL);
    for(auto& enemy: enemies) {
        if (enemy->IsDead()) continue;
        enemy->SetDetectionReduction(inventory.HasAura(ItemEffect::GraveShade) ? 1 : 0);
        EnemyAction action = enemy->Update(enemy_board, spawn_snake, detached_soldiers, interval);
        if (!action.attack_zone.empty()) {
            actions_list.push_back(action);
        }
    }
    // Observe after enemy movement; collect both sides before applying damage.
    std::vector<SoldierAction> soldier_actions;
    for (std::size_t i = 1; i < spawn_snake.body.size(); ++i) {
        SoldierAction action = spawn_snake.body[i].UpdateCombat(enemies, true, interval);
        if (inventory.HasAura(ItemEffect::HollowNight) && spawn_snake.body[i].hp == 1) {
            if (action.damage > 0) ++action.damage;
            for (auto& shot : action.projectiles) ++shot.damage;
        }
        if (!action.attack_zone.empty() || !action.projectiles.empty()) soldier_actions.push_back(action);
    }
    for (auto& soldier : detached_soldiers) {
        SoldierAction action = soldier.UpdateCombat(enemies, false, interval);
        if (inventory.HasAura(ItemEffect::HollowNight) && soldier.hp == 1) {
            if (action.damage > 0) ++action.damage;
            for (auto& shot : action.projectiles) ++shot.damage;
        }
        if (!action.attack_zone.empty() || !action.projectiles.empty()) soldier_actions.push_back(action);
    }
    // process enemy attack
    for (const auto& action : actions_list) {
        ProcessEnemyAttack(action);
        if (state.currentScreen == GAMEOVER) {
            return;
        }
    }

    // Existing arrows move first; new arrows begin traveling next fixed update.
    UpdateProjectiles(interval);
    // Released strikes survive an attacker's death, unless the leader's death ended the run.
    for (const auto& action : soldier_actions) ProcessSoldierAttack(action);

    UpdateBloomAttacks(interval);
    // remove dead enemies
    CleanupDeadEnemies();
    UpdateRecruitSpawning(interval);
}

void game::ProcessSoldierAttack(const SoldierAction& action) {
    if (state.currentScreen != STAGE) return;
    if (action.damage > 0) {
        for (const auto& cell : action.attack_zone) DamageEnemyAt(cell, action.damage);
    }
    for (const auto& spawn : action.projectiles) {
        if (spawn.damage <= 0 || spawn.speed <= 0.0f || spawn.max_distance <= 0 ||
            std::abs(spawn.direction.x) > 1 || std::abs(spawn.direction.y) > 1 ||
            spawn.direction == GridPosition{} || BlocksProjectile(spawn.position)) continue;
        projectiles.emplace_back(spawn);
    }
}

// Arrow terrain differs from walking: gaps can be crossed, empty/outside cells cannot.
bool game::BlocksProjectile(GridPosition cell) const {
    if (!board.IsInside(cell)) return true;
    CellType type = board.GetCellType(cell);
    return type == WALL || type == NONE || SpellBlocksEnemy(cell);
}

// Resolve one enemy hit without erasing during attack processing.
bool game::DamageEnemyAt(GridPosition cell, int damage, float slow_duration) {
    for (auto& enemy : enemies) {
        if (!enemy->IsDead() && enemy->GetPosition() == cell) {
            enemy->TakeDamage(damage);
            if (slow_duration > 0 && !enemy->IsDead()) enemy->ApplySlow(slow_duration);
            return true;
        }
    }
    return false;
}

// Check every crossed cell so fast arrows cannot skip obstacles or victims.
void game::UpdateProjectiles(float delta_time) {
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        bool finished = it->remaining_steps <= 0 || BlocksProjectile(it->position);
        if (!finished && it->has_moved) finished = DamageEnemyAt(it->position, it->damage, it->slow_duration);
        it->movement_progress += it->speed * delta_time;
        while (!finished && it->movement_progress >= 1.0f) {
            GridPosition next = {it->position.x + it->direction.x,
                                 it->position.y + it->direction.y};
            bool blocked_corner = it->direction.x != 0 && it->direction.y != 0 &&
                (BlocksProjectile({next.x, it->position.y}) ||
                 BlocksProjectile({it->position.x, next.y}));
            if (blocked_corner || BlocksProjectile(next)) {
                finished = true;
                break;
            }
            it->position = next;
            it->has_moved = true;
            it->movement_progress -= 1.0f;
            --it->remaining_steps;
            finished = DamageEnemyAt(next, it->damage, it->slow_duration) || it->remaining_steps <= 0;
        }
        if (finished) it = projectiles.erase(it);
        else ++it;
    }
}

void game::DrawSoldierAttacks() const {
    for (std::size_t i = 1; i < spawn_snake.body.size(); ++i) {
        spawn_snake.body[i].DrawAttack();
    }
    for (const auto& soldier : detached_soldiers) {
        if (soldier.CanAttack(false)) soldier.DrawAttack();
    }
}

void game::ProcessEnemyAttack(const EnemyAction& action) {
    if (state.currentScreen != STAGE) return;

    for (const auto& cell : action.attack_zone) {
        const bool attached_at_cell = std::any_of(
            spawn_snake.body.begin(), spawn_snake.body.end(),
            [&](const Soldier& soldier) { return soldier.position == cell; });
        if (attached_at_cell) {
            std::vector<Soldier> detached = spawn_snake.DamageNode(cell, action.damage);
            detached_soldiers.insert(detached_soldiers.end(), detached.begin(), detached.end());
        } else {
            DamageDetachedSoldier(cell, action.damage);
        }

        if (spawn_snake.IsDead()) {
            state.currentScreen = GAMEOVER;
            return;
        }
    }
}

void game::DamageDetachedSoldier(GridPosition position, int damage) {
    if (damage <= 0) return;

    for (auto it = detached_soldiers.begin(); it != detached_soldiers.end(); ++it) {
        if (it->hp <= 0 || !(it->position == position)) continue;
        if (it->ghost_time > 0.0f) return;
        if (it->shield_charge) {
            it->shield_charge = false;
            it->ghost_time = Soldier::initial_ghost_time;
            return;
        }

        it->hp -= damage;
        if (it->hp <= 0) {
            detached_soldiers.erase(it);
        } else {
            it->ghost_time = Soldier::initial_ghost_time;
        }
        return; // One victim per cell, including when that victim dies.
    }
}

void game::ReduceDetachedGhostTime(float delta_time) {
    for (auto& soldier : detached_soldiers) {
        soldier.ghost_time = std::max(0.0f, soldier.ghost_time - delta_time);
    }
}

void game::HandleSoldierRescue() {
    const GridPosition head_pos = spawn_snake.GetHeadPosition();
    for (auto it = detached_soldiers.begin(); it != detached_soldiers.end(); ++it) {
        if (it->hp > 0 && it->position == head_pos) {
            if (spawn_snake.AttachSoldier(*it)) {
                detached_soldiers.erase(it);
            }
            // Rescue at most one soldier per leader movement step.
            return;
        }
    }
}

// Reservoir sampling selects uniformly among valid cells without random retry loops.
bool game::FindRecruitSpawnCell(GridPosition& cell) const {
    int candidates = 0;
    for (int x = 0; x < board.GetWidth(); ++x) {
        for (int y = 0; y < board.GetHeight(); ++y) {
            GridPosition candidate = {x, y};
            if (board.GetCellType(candidate) != GROUND) continue;
            if (healing_shard_active && candidate == healing_shard_position) continue;
            if (SpellBlocksEnemy(candidate)) continue;
            if (std::any_of(world_spells.begin(), world_spells.end(), [candidate](const WorldSpell& spell) {
                return spell.effect == ItemEffect::GraveBloom && spell.cells.front() == candidate;
            })) continue;
            if (std::any_of(ground_items.begin(), ground_items.end(),
                [&](const GroundItem& item) { return item.position == candidate; })) continue;
            bool occupied = std::any_of(spawn_snake.body.begin(), spawn_snake.body.end(),
                [&](const Soldier& soldier) { return soldier.position == candidate; });
            occupied = occupied || std::any_of(detached_soldiers.begin(), detached_soldiers.end(),
                [&](const Soldier& soldier) { return soldier.hp > 0 && soldier.position == candidate; });
            occupied = occupied || std::any_of(enemies.begin(), enemies.end(),
                [&](const std::unique_ptr<Enemy>& enemy) {
                    return !enemy->IsDead() && enemy->GetPosition() == candidate;
                });
            if (!occupied && GetRandomValue(1, ++candidates) == 1) cell = candidate;
        }
    }
    return candidates > 0;
}

// Extend this terrain multiplier when recruit-friendly terrain is introduced.
float game::GetRecruitSpawnChance(GridPosition cell) const {
    float terrain_multiplier = board.GetCellType(cell) == GROUND ? 1.0f : 0.0f;
    return std::max(0.0f, std::min(1.0f,
        recruit_spawn.base_chance * terrain_multiplier * recruit_spawn.skill_multiplier));
}

// One scheduled attempt, independent of pickup/death; blocked attempts never build a backlog.
void game::UpdateRecruitSpawning(float delta_time) {
    if (recruit_spawn.interval <= 0.0f) {
        recruit_spawn_elapsed = 0.0f;
        return;
    }
    recruit_spawn_elapsed += std::max(0.0f, delta_time);
    if (recruit_spawn_elapsed < recruit_spawn.interval) return;
    recruit_spawn_elapsed = 0.0f;

    if (!spawn_snake.HasFollowerCapacity()) return;
    const auto waiting = std::count_if(detached_soldiers.begin(), detached_soldiers.end(),
        [](const Soldier& soldier) { return soldier.hp > 0; });
    if (static_cast<std::size_t>(waiting) >= recruit_spawn.max_waiting) return;

    GridPosition cell;
    if (!FindRecruitSpawnCell(cell)) return;
    if (GetRandomValue(0, 9999) >= GetRecruitSpawnChance(cell) * 10000.0f) return;

    int total_weight = 0;
    for (const auto& entry : recruit_spawn.type_weights) total_weight += std::max(0, entry.second);
    if (total_weight <= 0) return;
    int roll = GetRandomValue(1, total_weight);
    for (const auto& entry : recruit_spawn.type_weights) {
        roll -= std::max(0, entry.second);
        if (roll <= 0) {
            Soldier recruit(entry.first);
            recruit.position = cell;
            detached_soldiers.push_back(recruit);
            return;
        }
    }
}

void game::CleanupDeadEnemies() {
    enemies.erase(
        std::remove_if(
            enemies.begin(),
            enemies.end(),
            [](const std::unique_ptr<Enemy>& enemy) {
                return enemy->IsDead();
            }
        ),
        enemies.end()
    );
}



