#pragma once

#include "common.h"
#include "room_template.h"
#include "board.h"

struct ObjectMarker {
    std::string name;
    GridPosition position;
};

class Room {
    public:
        bool Load(const RoomTemplate& _template, Board& _board);
        GridPosition GetEntrance() const;
        DoorSide GetDoorSide(GridPosition door_pos) const;
        const std::vector<GridPosition>& GetExit() const;
    private:
        int width = 0;
        int height = 0;
        bool TryGetCellType(std::string cell_name, CellType& output_type);
        GridPosition entrance;
        std::vector<GridPosition> exits;
        std::vector<ObjectMarker> object_markers;
    };