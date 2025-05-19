#include "Player.h"
#include "Bullet.h"
#include <iostream>
#include <chrono>

Player::Player(int startX, int startY, int moveSpeed)
    : lives(3), SHOT_COOLDOWN(500) {
    x = startX;
    y = startY;
    speed = moveSpeed;
    lastShotTime = -SHOT_COOLDOWN;
}

void Player::update() {}

void Player::moveLeft() {
    x -= speed;
    if (x <= 2) x = 2;
}

void Player::moveRight() {
    x += speed;
    if (x >= SCREEN_WIDTH - 2) x = SCREEN_WIDTH - 2;
}

void Player::draw() {
    std::cout << "\033[" << y << ";" << x << "H" << BLUE "=" RESET;
}

void Player::shoot(std::vector<Entity*>& entities) {
    int currentTime = getCurrentTime();
    if (currentTime - lastShotTime >= SHOT_COOLDOWN) {
        entities.push_back(new Bullet(x, y - 1, BULLET_SPEED, 1, 1));
        lastShotTime = currentTime;
    }
}

int Player::getCurrentTime() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

bool Player::isPlayer() const {
    return true;
}

void Player::takeDamage() {
    lives--;
    if (lives <= 0) {
        std::cout << "\033[" << SCREEN_HEIGHT/2 << ";" << SCREEN_WIDTH/2-5 << "HGame Over!" << std::endl;
        std::cout << "\033[" << SCREEN_HEIGHT/2+1 << ";" << SCREEN_WIDTH/2-6 << "HPress any key to exit..." << std::endl;
        Terminal::disableRawMode();
        while (!Terminal::kbhit()) usleep(100000);
        std::cout << "\033[?25h";
        Terminal::disableRawMode();
        while (!Terminal::kbhit()) usleep(100000);
        getchar();
        exit(0);
    }
}

int Player::getLives() const {
    return lives;
}
