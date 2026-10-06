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

