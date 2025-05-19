#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "Entity.h"

class Explosion : public Entity {
private:
    int lifeTime = 5;

public:
    Explosion(int startX, int startY);
    void update() override;
    void draw() override;
    bool isExplosion() const override;
    int getLifeTime() const;
};

#endif // EXPLOSION_H
