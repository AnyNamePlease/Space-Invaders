#include "Barrier.h"
#include <iostream>

using namespace std;

void Barrier::update() {}

void Barrier::draw() {
    if (health > 0)
        cout << "\033[" << y << ";" << x << "H"
             << (health == 3 ? GREEN "###" RESET :
                 (health == 2 ? GREEN "## " RESET : GREEN "#  " RESET));
}
