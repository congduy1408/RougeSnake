#pragma once
#include "common.h"
#include "game_textures.h"

enum CellType {
    NONE,
    GROUND,
    WALL,
    GAP,
    ENTRANCE,
    DOOR
};

struct BoardCell {
    CellType type = NONE;
};

class Board {
    public:
        Board(int width, int height);
        void Initialize(int width, int height);
        void Reset();
        bool IsInside(GridPosition position) const;
        void Draw(const GameTextures& textures);
        int GetWidth() const;
        int GetHeight() const;

        // implement board cell
        void SetCell(GridPosition position, CellType type);
        CellType GetCellType(GridPosition position) const;
        bool IsWalkable(GridPosition position) const;
        bool IsLethal(GridPosition position) const;
        bool IsDoor(GridPosition position) const;
    private:
        int width;
        int height;

        // implement Board cell
        std::vector<std::vector<BoardCell>> cells;
};