#include "Enemy.h"
#include "Bullet.h"
#include <iostream>
#include <cstdlib>


void Enemy::update() {}

void Enemy::draw() {
    char symbol = ' ';
    switch (type) {
        case 0: symbol = 'O'; break;
        case 1: symbol = '#'; break;
        case 2: symbol = 'W'; break;
    }
    cout << "\033[" << y << ";" << x << "H" << GREEN << symbol << RESET;
}

void Enemy::shoot(vector<Entity*>& entities) {
    if (rand() % 1012 == 0) {
        entities.push_back(new Bullet(x, y + 1, BULLET_SPEED, -1, -1));
    }
}

void Enemy::takeDamage() {
    health--;
}
