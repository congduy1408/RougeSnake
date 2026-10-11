#include "include/room_template.h"
#include <fstream>
#include "include/nlohmann/json.hpp"

using json = nlohmann::json;

const std::string& RoomTemplate::GetName() const {
    return name;
}

int RoomTemplate::GetWidth() const {
    return width;
}
int RoomTemplate::GetHeight() const {
    return height;
}
const std::vector<RoomLayer>& RoomTemplate::GetLayers() const{
    return layers;
}

bool RoomTemplate::LoadFromFile(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cout << "Failed to open room file: "
            << path << std::endl;
        return false;
    }

    try {
        json data;
        file >> data;

        std::string tmp_name = data["name"].get<std::string>();
        int tmp_width = data["size"]["width"].get<int>();
        int tmp_height = data["size"]["height"].get<int>();

        std::vector<RoomLayer> tmp_layers;
        for (const auto& layerJson : data["layers"]) {
            RoomLayer layer;
            layer.name = layerJson["name"].get<std::string>();
            for (const auto& cellJson : layerJson["cells"]) {
                RoomCellData cell;
                cell.name = cellJson["name"].get<std::string>();
                cell.position = GridPosition{
                    cellJson["x"].get<int>(),
                    cellJson["y"].get<int>()
                };
                if (cell.position.x >= tmp_width ||
                    cell.position.x < 0 ||
                    cell.position.y >= tmp_height ||
                    cell.position.y < 0 
                ) {
                    std::cout << "Incorrect cell position: "
                    << cell.position.x << ":" << cell.position.y << std::endl;
                    return false;
                }
                layer.cells.push_back(cell);
            }
            tmp_layers.push_back(layer);
        }
        if (tmp_width <= 0 || tmp_height <=0) {
            std::cout << "Incorrect map dimension: "
            << tmp_width << ":" << tmp_height << std::endl;
            return false;
        }
        name = tmp_name;
        width = tmp_width;
        height = tmp_height;
        layers = tmp_layers;
    }
    catch (const std::exception& e) {
        std::cout << "Failed to load room: "
            << e.what() << std::endl;
        return false;
    }

    return true;
}