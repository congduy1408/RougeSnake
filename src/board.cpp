#include "include/board.h"

Board::Board(int board_width, int board_height)
    :width(board_width),
    height(board_height) {
        Reset();
    }

void Board::Reset() {
    walls.clear();
    blocked_cells.assign(width * height, false);
    CreateBoundaryWalls();
}

void Board::Draw() const {
    for (const Brick& wall : walls)
    {
        wall.Draw();
    }
}

bool Board::IsInside(GridPosition position) const {
    return position.x >= 0 &&
        position.x < width &&
        position.y >= 0 &&
        position.y < height;
}

bool Board::IsBlocked(GridPosition position) const {
    if (!IsInside(position)) {
        return true;
    }
    return blocked_cells[ToIndex(position)];
}

int Board::GetHeight() {
    return height;
}

int Board::GetWidth() {
    return width;
}

void Board::AddWall(GridPosition position) {
    if (!IsInside(position)) {
        return;
    }
    int index = ToIndex(position);
    if (blocked_cells[index]) {
        return;
    }
    blocked_cells[index] = true;
    walls.emplace_back(position);
}

int Board::ToIndex(GridPosition position) const {
    return position.y * width + position.x;
}

void Board::CreateBoundaryWalls() {
    for (int x=0; x<width; x++) {
        AddWall(GridPosition{x, 0});
        AddWall(GridPosition{x,height-1});
    }
    for (int y=0; y<height; y++) {
        AddWall(GridPosition{0, y});
        AddWall(GridPosition{width-1, y});
    }
}