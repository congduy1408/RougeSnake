#pragma once

#include "common.h"

class Snake;
class GameObject {
    protected:
        GridPosition position = {0,0};
    public:
        GameObject(GridPosition startPosition)
        : position (startPosition)
        {
        }
        GameObject() = default;
        void SetPosition(GridPosition _position) {
            position = _position;
        }
        GridPosition GetPosition () const{
            return position;
        }
        bool IsAt (GridPosition other) {
            return position == other;
        }
        virtual ~GameObject() = default;
        virtual void Update() {};
        virtual void OnSnakeEnter(Snake& snake) {};
};