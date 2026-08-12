#pragma once

class Snake;
class GameObject {
    protected:
        Vector2 position = {0,0};
    public:
        GameObject(Vector2 startPosition)
        : position (startPosition)
        {
        }
        GameObject() = default;
        void SetPosition(Vector2 _position) {
            position = _position;
        }
        Vector2 GetPosition (){
            return position;
        }
        virtual ~GameObject() = default;
        virtual void Update() {};
        virtual void OnSnakeEnter(Snake& snake) {};
};