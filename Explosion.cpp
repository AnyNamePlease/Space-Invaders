#include "Explosion.h"
#include <iostream>


void Explosion::update() {
    lifeTime--;
}

void Explosion::draw() {
    if (lifeTime > 0) {
        cout << "\033[" << y << ";" << x << "H"
             << (lifeTime % 2 == 0 ? RED "*" RESET : RED "#" RESET);
    }
}
