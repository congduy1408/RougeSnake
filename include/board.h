#pragma once
#include "common.h"
#include "wall.h"

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

        void Reset();
        bool IsInside(GridPosition position) const;
        void AddCell(GridPosition position, CellType type);
        void Draw() const;
        int GetWidth();
        int GetHeight();

        // implement board cell
        void SetCell(GridPosition position, CellType type);
        CellType GetCellType(GridPosition position);
        bool IsWalkable(GridPosition position);
        bool IsLethal(GridPosition position);
        bool IsDoor(GridPosition position);
    private:
        int width;
        int height;

        std::vector<Brick> walls;

        // implement Board cell
        std::vector<std::vector<BoardCell>> cells;
        
        void CreateBoundaryWalls();
        // test add ground
        void CreateGround();

};