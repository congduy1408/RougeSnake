#include "include/item.h"
#include <cassert>
#include <set>

int main() {
    const auto& catalog = GetFillerItems();
    const std::string expected_letters = "ABCDEF MNOPQRST";
    assert(catalog.size() == 14);
    std::set<std::string> ids;
    for (const auto& item : catalog) {
        assert(ids.insert(item.id).second);
        assert(!item.name.empty() && !item.description.empty());
        assert(item.category == ItemCategory::Filler && item.effect == ItemEffect::None);
        assert(item.cooldown == 0 && item.cooldown_remaining == 0);
        assert(expected_letters.find(item.letter) != std::string::npos);
        LeaderInventory inventory;
        inventory.items.push_back(item);
        assert(!inventory.UseSelected());
        assert(inventory.items[0].cooldown_remaining == 0);
        assert(inventory.items[0].id == item.id); // Passive use never consumes the item.
    }
    LeaderInventory inventory;
    inventory.cipher = "MOON";
    assert(inventory.Capacity() == 6);
    inventory.items = {catalog[6], catalog[8], catalog[7]}; // M, O, N
    assert(inventory.CipherMatches() == std::vector<bool>({true, true, false, true}));
    inventory.items.push_back(catalog[8]);
    assert(inventory.CipherMatches() == std::vector<bool>({true, true, true, true}));
    inventory.items.erase(inventory.items.begin() + 1);
    assert(inventory.CipherMatches() == std::vector<bool>({true, true, false, true}));
    inventory.selected = 0;
    inventory.Cycle(-1);
    assert(inventory.selected == 2);
    inventory.Cycle(1);
    assert(inventory.selected == 0);
    Item dropped = inventory.items[0];
    inventory.items[0] = catalog[0]; // Replace M with A.
    assert(dropped.id == "moonlit_talisman");
    assert(!inventory.CipherMatches()[0]);
    assert(GetFillerItems()[6].id == "moonlit_talisman"); // Catalog isn't mutated by instances.
}
