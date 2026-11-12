#include "include/item.h"

const std::vector<Item>& GetFillerItems() {
    static const std::vector<Item> items = {
        {"ashen_talisman", "Ashen Talisman", ItemCategory::Filler, 'A', "A small talisman coated in cold ash."},
        {"bone_pebble", "Bone Pebble", ItemCategory::Filler, 'B', "A pale stone worn smooth among old bones."},
        {"cryptbound_book", "Cryptbound Book", ItemCategory::Filler, 'C', "A sealed book bound in weathered leather."},
        {"dustvein_amulet", "Dustvein Amulet", ItemCategory::Filler, 'D', "Fine trails of dust fill its silver veins."},
        {"elder_charm", "Elder Charm", ItemCategory::Filler, 'E', "An ancient charm polished by countless hands."},
        {"forgotten_ring", "Forgotten Ring", ItemCategory::Filler, 'F', "Its inscription has faded beyond recognition."},
        {"moonlit_talisman", "Moonlit Talisman", ItemCategory::Filler, 'M', "A cool silver token with a crescent-shaped mark."},
        {"nightgrave_pebble", "Nightgrave Pebble", ItemCategory::Filler, 'N', "A dark stone gathered from a silent grave."},
        {"ossuary_book", "Ossuary Book", ItemCategory::Filler, 'O', "A brittle volume lined with sketches of bones."},
        {"pale_amulet", "Pale Amulet", ItemCategory::Filler, 'P', "A faded pendant hanging from a frayed cord."},
        {"quiet_charm", "Quiet Charm", ItemCategory::Filler, 'Q', "A tiny bell that no longer makes a sound."},
        {"rusted_ring", "Rusted Ring", ItemCategory::Filler, 'R', "A weathered iron band stained with rust."},
        {"silent_talisman", "Silent Talisman", ItemCategory::Filler, 'S', "A folded ward secured with black thread."},
        {"tomb_pebble", "Tomb Pebble", ItemCategory::Filler, 'T', "A chipped stone from an old tomb's threshold."}
    };
    return items;
}

void LeaderInventory::Cycle(int step) {
    if (items.empty()) { selected = 0; return; }
    const int count = static_cast<int>(items.size());
    selected = static_cast<std::size_t>(((static_cast<int>(selected % items.size()) + step) % count + count) % count);
}

void LeaderInventory::UpdateCooldowns(float delta_time) {
    if (delta_time <= 0.0f) return;
    for (auto& item : items)
        item.cooldown_remaining = std::max(0.0f, item.cooldown_remaining - delta_time);
}

bool LeaderInventory::UseSelected() {
    if (selected >= items.size() || items[selected].effect == ItemEffect::None ||
        items[selected].cooldown_remaining > 0.0f) return false;
    items[selected].cooldown_remaining = std::max(0.0f, items[selected].cooldown);
    return true;
}

std::vector<bool> LeaderInventory::CipherMatches() const {
    std::vector<bool> matched(cipher.size(), false);
    std::vector<bool> used(items.size(), false);
    for (std::size_t i = 0; i < cipher.size(); ++i) {
        for (std::size_t j = 0; j < items.size(); ++j) {
            if (!used[j] && items[j].letter == cipher[i]) {
                matched[i] = true;
                used[j] = true;
                break;
            }
        }
    }
    return matched;
}
