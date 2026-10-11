#pragma once

#include "common.h"
#include "room_template.h"
#include "board.h"

class Room {
    public:
        void Load(const RoomTemplate& _template, Board& _board);
    private:
        bool TryGetCellType(std::string cell_name, CellType& output_type);
};