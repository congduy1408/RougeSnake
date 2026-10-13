#pragma once

#include "common.h"
#include "room_template.h"
#include "board.h"

class Room {
    public:
        void Load(const RoomTemplate& _template, Board& _board);
        GridPosition GetEntrance() const;
        const std::vector<GridPosition>& GetExit() const;
    private:
        bool TryGetCellType(std::string cell_name, CellType& output_type);
        GridPosition entrance;
        std::vector<GridPosition> exits;
    };