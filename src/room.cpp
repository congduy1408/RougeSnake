#include "include/room.h"

void Room::Load(const RoomTemplate& _template, Board& _board) {
    // Init board
    _board.Initialize(_template.GetWidth(), _template.GetHeight());
    // reading room template
    for (const auto& layer : _template.GetLayers()) {
        for (const auto& cell : layer.cells) {
            // get cell name + check cell type
            CellType type;
            bool get_result = TryGetCellType(cell.name, type);
            // get cell position
            if (get_result) {
                _board.SetCell(cell.position, type);
            }
        }
    }
}

bool Room::TryGetCellType(std::string cell_name, CellType& output_type) {
    if (cell_name == "wall") {
        output_type = WALL;
        return true;
    }
    else if (cell_name == "ground") {
        output_type = GROUND;
        return true;
    }
    else if (cell_name == "gap") {
        output_type = GAP;
        return true;
    }
    else if (cell_name == "entrance") {
        output_type = ENTRANCE;
        return true;
    }
    else if (cell_name == "door") {
        output_type = DOOR;
        return true;
    } 
    else {
        return false;
    }
}