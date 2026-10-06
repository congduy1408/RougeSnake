#include "include/board.h"

Board::Board(int board_width, int board_height)
    :width(board_width),
    height(board_height) {
        Reset();
    }

void Board::Reset() {
    walls.clear();
    // implement board cell
    cells = std::vector<std::vector<BoardCell>> (width, std::vector<BoardCell>(height));
    CreateBoundaryWalls();
    CreateGround();
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

int Board::GetHeight() {
    return height;
}

int Board::GetWidth() {
    return width;
}

void Board::AddCell(GridPosition position, CellType type) {
    if (!IsInside(position)) {
        return;
    }

    // for Draw
    switch (type) {
        case WALL:
            walls.emplace_back(position);
            break;
        default:
            break;
    }

    SetCell(position, type);
}

void Board::CreateBoundaryWalls() {
    for (int x=0; x<width; x++) {
        AddCell(GridPosition{x, 0}, WALL);
        AddCell(GridPosition{x,height-1}, WALL);
    }
    for (int y=0; y<height; y++) {
        AddCell(GridPosition{0, y}, WALL);
        AddCell(GridPosition{width-1, y}, WALL);
    }
}

void Board::CreateGround() {
    for (int x=1;x<width-1;x++) {
        for (int y=1;y<height-1; y++) {
            AddCell(GridPosition{x,y},GROUND);
        }
    }
}

void Board::SetCell(GridPosition position, CellType type) {
    if (!IsInside(position)) {
        return;
    }
    cells[position.x][position.y].type = type;
}

CellType Board::GetCellType(GridPosition position) {
    if (!IsInside(position)) {
        return NONE;
    }
    return cells[position.x][position.y].type;
}

bool Board::IsWalkable(GridPosition position) {
    CellType _type = GetCellType(position);
    switch(_type) {
        case NONE:
            return  false;
        case GROUND:
            return true;
        case WALL:
            return false;
        case GAP:
            return true;
        case ENTRANCE:
            return false;
        case DOOR:
            return true;
        default:
            return false;
    }
}

bool Board::IsLethal(GridPosition position) {
    CellType _type = GetCellType(position);
    switch(_type) {
        case NONE:
            return  false;
        case GROUND:
            return false;
        case WALL:
            return false;
        case GAP:
            return true;
        case ENTRANCE:
            return false;
        case DOOR:
            return false;
        default:
            return false;
    }
}
bool Board::IsDoor(GridPosition position) {
    CellType _type = GetCellType(position);
    switch(_type) {
        case NONE:
            return  false;
        case GROUND:
            return false;
        case WALL:
            return false;
        case GAP:
            return false;
        case ENTRANCE:
            return false;
        case DOOR:
            return true;
        default:
            return false;
    }
}
