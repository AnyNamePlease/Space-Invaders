#ifndef BULLET_H
#define BULLET_H

#include "Entity.h"

class Bullet : public Entity {
private:
    int direction;
    int owner;

public:
    Bullet(int startX, int startY, int moveSpeed, int dir, int own);
    void update() override;
    void draw() override;
    bool is_off_screen();
    int getOwner() const;
};

#endif // BULLET_H
