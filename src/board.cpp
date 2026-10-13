#include "include/board.h"

Board::Board(int board_width, int board_height)
    :width(board_width),
    height(board_height) {
        Reset();
    }

void Board::Initialize(int init_width, int init_height) {
    width = init_width;
    height = init_height;
    cells = std::vector<std::vector<BoardCell>> (width, std::vector<BoardCell>(height));
}

void Board::Reset() {
    Initialize(cellcount_width, cellcount_height);
}

void Board::Draw(const GameTextures& textures) {
    // for (const Brick& wall : walls)
    // {
    //     wall.Draw();
    // }
    Texture2D sprite = textures.GetTexture();
    for(int x=0; x< cells.size(); x++) {
        for (int y=0; y <cells[x].size(); y++) {
            Rectangle draw_rect;
            switch (cells[x][y].type) {
                case GROUND:
                    draw_rect = textures.GetSpriteRect(SpriteID::GROUND);
                    break;
                case WALL:
                    draw_rect = textures.GetSpriteRect(SpriteID::WALL);
                    break;
                case GAP:
                    draw_rect = textures.GetSpriteRect(SpriteID::GAP);
                    break;
                case ENTRANCE:
                    draw_rect = textures.GetSpriteRect(SpriteID::ENTRANCE);
                    break;
                case DOOR:
                    draw_rect = textures.GetSpriteRect(SpriteID::DOOR);
                    break;
                case NONE:
                    continue;
            }
            Rectangle draw_dest = {
                static_cast<float> (x*cellsize), 
                static_cast<float> (y*cellsize),
                static_cast<float> (cellsize),
                static_cast<float> (cellsize)};
            Vector2 origin = {0, 0};
            float rotation = 0;
            DrawTexturePro(
                sprite,
                draw_rect,
                draw_dest,
                origin,
                rotation,
                WHITE
            );
        }
    }

}

bool Board::IsInside(GridPosition position) const {
    return position.x >= 0 &&
        position.x < width &&
        position.y >= 0 &&
        position.y < height;
}

int Board::GetHeight() const {
    return height;
}

int Board::GetWidth() const {
    return width;
}

void Board::SetCell(GridPosition position, CellType type) {
    if (!IsInside(position)) {
        return;
    }
    if (cells[position.x][position.y].type == ENTRANCE) return;
    if (cells[position.x][position.y].type == DOOR) return;
    cells[position.x][position.y].type = type;
}

CellType Board::GetCellType(GridPosition position) const {
    if (!IsInside(position)) {
        return NONE;
    }
    return cells[position.x][position.y].type;
}

bool Board::IsWalkable(GridPosition position) const {
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

bool Board::IsLethal(GridPosition position) const {
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
bool Board::IsDoor(GridPosition position) const {
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
