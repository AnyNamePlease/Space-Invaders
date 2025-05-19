#ifndef GAME_H
#define GAME_H

#include <vector>
#include "Entity.h"

class Game {
private:
    std::vector<Entity*> entities;
    class Player* player;
    int currentWave = 1;
    bool waveActive = true;
    int enemyDirection = 1;
    int score = 0;
    bool isPaused = false;

    void spawnEnemies(int waveNumber);
    int countEnemies() const;
    void startNextWave();
    void displayInfo() const;
    void checkFleetMovement();
    void checkPlayerDefeat();
    void showStartScreen();
    void pauseGame();

public:
    void run();
    void handleEvents();
    void update();
    void draw();
    void addScore(int points) { score += points; }
};

#endif // GAME_H
