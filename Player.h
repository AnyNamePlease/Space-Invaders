#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include <chrono>

class Player : public Entity {
private:
    int lives = 3;
    const int SHOT_COOLDOWN = 500;
    int lastShotTime = -SHOT_COOLDOWN;

public:
    Player(int startX, int startY, int moveSpeed);
    void update() override;
    void moveLeft();
    void moveRight();
    void draw() override;
    void shoot(std::vector<Entity*>& entities);
    int getCurrentTime() const;
    bool isPlayer() const override;
    void takeDamage();
    int getLives() const;
};

#endif // PLAYER_H
