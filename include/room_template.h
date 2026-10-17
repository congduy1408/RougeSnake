#pragma once
#include "common.h"

struct RoomCellData {
    std::string name;
    GridPosition position;
};

struct RoomLayer {
    std::string name;
    std::vector<RoomCellData> cells;
};

class RoomTemplate {

    std::string name;
    int width;
    int height;
    std::vector<RoomLayer> layers;

    public:
        const std::string& GetName() const;
        int GetWidth() const;
        int GetHeight() const;
        const std::vector<RoomLayer>& GetLayers()const;
        bool LoadFromFile(const std::string& path);
};


class RoomCatalog {
    private:
        std::vector<std::string> room_left;
        std::vector<std::string> room_right;
        std::vector<std::string> room_up;
        std::vector<std::string> room_down;
    public:
        bool LoadAll(const std::string& manifest_path);
        std::string SelectRandom(
            DoorSide required_entrance
        ) const;
};
