#ifndef ENTITY_H
#define ENTITY_H

class Entity {
protected:
    int x;
    int y;
    int speed;

public:
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual bool isEnemy() const { return false; }
    virtual bool isPlayer() const { return false; }
    virtual bool isExplosion() const { return false; }
    virtual ~Entity() {}
    int getX() const { return x; }
    int getY() const { return y; }
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    void moveX(int dx) { x += dx; }
    void moveY(int dy) { y += dy; }
};

#endif // ENTITY_H
