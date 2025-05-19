#include "Bullet.h"
#include <iostream>


void Bullet::update() {
    y -= speed * direction;
}

void Bullet::draw() {
    cout << "\033[" << y << ";" << x << "H"
         << (owner == 1 ? GREEN "*" RESET : RED "*" RESET);
}
