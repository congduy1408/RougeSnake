#pragma once
#include "common.h"
#include "wall.h"
class Board {
    public:
        Board(int width, int height);

        void Reset();
        bool IsInside(GridPosition position) const;
        bool IsBlocked(GridPosition position) const;
        void AddWall(GridPosition position);
        void Draw() const;
        int GetWidth();
        int GetHeight();
    private:
        int width;
        int height;

        std::vector<bool> blocked_cells;
        std::vector<Brick> walls;

        int ToIndex(GridPosition position) const;
        void CreateBoundaryWalls();

};