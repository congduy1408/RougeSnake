#include "include/room.h"

bool Room::Load(const RoomTemplate& _template, Board& _board) {
    // Init board
    exits.clear();
    object_markers.clear();
    width = _template.GetWidth();
    height = _template.GetHeight();
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
                if (type == ENTRANCE) {
                    entrance = cell.position;
                    if (GetDoorSide(cell.position) == DoorSide::INVALID) return false;
                }
                if (type == DOOR) { 
                    exits.push_back(cell.position);
                    if (GetDoorSide(cell.position) == DoorSide::INVALID) return false;
                } 
            } else {
                object_markers.push_back({cell.name, cell.position});
            }

        }
    }
    return true;
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

GridPosition Room::GetEntrance() const {
    return entrance;
}
const std::vector<GridPosition>& Room::GetExit() const {
    return exits;
}

DoorSide Room::GetDoorSide(GridPosition door_pos) const {
    if (door_pos.x < 0 || door_pos.x >= width ||
        door_pos.y < 0 || door_pos.y >= height) {
        return DoorSide::INVALID;
    }

    bool on_vertical_edge =
        door_pos.x == 0 || door_pos.x == width - 1;

    bool on_horizontal_edge =
        door_pos.y == 0 || door_pos.y == height - 1;

    if (on_vertical_edge == on_horizontal_edge) {
        return DoorSide::INVALID;
    }

    if (door_pos.x == 0) { 
        return DoorSide::LEFT;
    } else if (door_pos.x == width - 1) { 
        return DoorSide::RIGHT;
    } else if (door_pos.y == 0) { 
        return DoorSide::UP;
    } else if (door_pos.y == height - 1) { 
        return DoorSide::DOWN;
    } else {
        return DoorSide::INVALID;
    }
}