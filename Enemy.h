#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

class Enemy : public Entity {
private:
    int type;
    int health;
    int direction;

public:
    Enemy(int startX, int startY, int moveSpeed, int t);
    void update() override;
    void draw() override;
    void shoot(std::vector<Entity*>& entities);
    void takeDamage();
    bool isDestroyed() const;
    bool isEnemy() const override;
    bool isHit(int bulletX, int bulletY) const;
};

#endif // ENEMY_H
