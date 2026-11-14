#include "include/item.h"
#include <cassert>

int main() {
    const Item& dawn = GetDawnVeil();
    assert(dawn.id == "dawn_veil" && dawn.category == ItemCategory::Aura && dawn.letter == 'D');
    LeaderInventory inventory;
    inventory.cipher = "DD";
    inventory.items = {dawn, dawn, GetFillerItems()[0]};
    inventory.selected = 2;
    assert(inventory.HasAura(ItemEffect::DawnVeil)); // Selecting a filler keeps the aura active.
    inventory.UpdateCooldowns(10000);
    assert(inventory.HasAura(ItemEffect::DawnVeil)); // No lifetime/expiry.
    inventory.selected = 0;
    assert(!inventory.UseSelected() && inventory.items[0].cooldown_remaining == 0);
    assert(inventory.CipherMatches() == std::vector<bool>({true, true}));
    inventory.items.erase(inventory.items.begin());
    assert(inventory.HasAura(ItemEffect::DawnVeil));
    assert(inventory.CipherMatches() == std::vector<bool>({true, false}));
    inventory.items.erase(inventory.items.begin());
    assert(!inventory.HasAura(ItemEffect::DawnVeil));
    assert(inventory.CipherMatches() == std::vector<bool>({false, false}));
    Item mislabeled = dawn;
    mislabeled.category = ItemCategory::Filler;
    inventory.items.push_back(mislabeled);
    assert(!inventory.HasAura(ItemEffect::DawnVeil));
}
