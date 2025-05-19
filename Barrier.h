#ifndef BARRIER_H
#define BARRIER_H

#include "Entity.h"

class Barrier : public Entity {
private:
    int health = 3;

public:
    Barrier(int startX, int startY);
    void update() override;
    void draw() override;
    void takeDamage();
    int getHealth() const;
    bool isEnemy() const override;
};

#endif // BARRIER_H
