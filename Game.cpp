#include "Game.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Barrier.h"
#include "Explosion.h"
#include "Terminal.h"
#include <iostream>
#include <vector>
#include <unordered_set>
#include <cstdlib>
#include <ctime>
#include <chrono>

void Game::showStartScreen() {
    system("clear");

    cout << "\n\033[" << SCREEN_HEIGHT/2 - 2 << ";" << SCREEN_WIDTH/2 - 10 << "H" << GREEN "SPACE INVADERS" RESET << endl;
    cout << "\n\033[" << SCREEN_HEIGHT/2 << ";" << SCREEN_WIDTH/2 - 10 << "HControls: A/D - Move, SPACE - Shoot, P - Pause" << endl;
    cout << "\n\033[" << SCREEN_HEIGHT/2 + 1 << ";" << SCREEN_WIDTH/2 - 10 << "HTry to survive all 5 waves!" << endl;
    cout << "\n\033[" << SCREEN_HEIGHT/2 + 2 << ";" << SCREEN_WIDTH/2 - 10 << "HPress any key to start..." << endl;

    // Ожидание нажатия
    while (!Terminal::kbhit()) usleep(100000);
    getchar(); // сброс нажатия
}

void Game::pauseGame() {
    isPaused = true;
    cout << "\n\033[" << SCREEN_HEIGHT/2 << ";" << SCREEN_WIDTH/2 - 5 << "HPAUSED" << endl;
    cout << "\033[" << SCREEN_HEIGHT/2 + 1 << ";" << SCREEN_WIDTH/2 - 6 << "HPress any key to continue..." << endl;

    while (isPaused && !Terminal::kbhit()) usleep(100000);
    isPaused = false;
}

void Game::run() {
    srand(time(0));
    showStartScreen();

    system("clear");
    cout << "\033[?25l"; // скрыть курсор

    spawnEnemies(currentWave);
    player = new Player(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 2, 2);
    entities.push_back(player);

    Terminal::enableRawMode();

    while (true) {
        draw();
        handleEvents();
        if (!isPaused) {
            update();
        }
        usleep(50000);
    }

    Terminal::disableRawMode();
}

void Game::handleEvents() {
    if (Terminal::kbhit()) {
        char input = getchar();
        if (isPaused) {
            isPaused = false;
            return;
        }
        if (input == 'a') player->moveLeft();
        else if (input == 'd') player->moveRight();
        else if (input == ' ') player->shoot(entities);
        else if (input == 'p' || input == 'P') {
            pauseGame();
        }
    }
}

void Game::update() {
    // Обновляем все сущности
    for (Entity* e : entities) {
        e->update();
    }

    // Враги стреляют
    for (Entity* e : entities) {
        if (e->isEnemy()) {
            Enemy* enemy = dynamic_cast<Enemy*>(e);
            if (enemy) enemy->shoot(entities);
        }
    }

    // Проверяем движение флота
    checkFleetMovement();

    // --- Обработка столкновений ---
    std::unordered_set<Entity*> toDelete;
    std::vector<Entity*> newObjects;

    // Обрабатываем столкновения пуль
    for (Entity* e : entities) {
        Bullet* bullet = dynamic_cast<Bullet*>(e);
        if (!bullet) continue;

        // Проверяем столкновения с другими объектами
        for (Entity* target : entities) {
            if (target == bullet) continue;
            
            // Проверка пересечения координат
            if (bullet->getX() == target->getX() && 
                abs(bullet->getY() - target->getY()) <= 1) {
                
                // Столкновение с врагом
                if (bullet->getOwner() == 1 && target->isEnemy()) {
                    Enemy* enemy = dynamic_cast<Enemy*>(target);
                    if (enemy) {
                        enemy->takeDamage();
                        if (enemy->isDestroyed()) {
                            toDelete.insert(bullet);
                            toDelete.insert(enemy);
                            addScore(10 + enemy->getY());
                            newObjects.push_back(new Explosion(enemy->getX(), enemy->getY()));
                        }
                    }
                    break;
                }
                
                // Столкновение с игроком
                else if (bullet->getOwner() == -1 && target->isPlayer()) {
                    toDelete.insert(bullet);
                    Player* p = dynamic_cast<Player*>(target);
                    if (p) p->takeDamage();
                    break;
                }
                
                // Столкновение с барьером
                else if (dynamic_cast<Barrier*>(target) && bullet->getOwner() != 1) { // Барьер не должен разрушаться от пуль игрока
                    Barrier* b = dynamic_cast<Barrier*>(target);
                    b->takeDamage();
                    toDelete.insert(bullet);
                    break;
                }
            }
        }
    }

    entities.insert(entities.end(), newObjects.begin(), newObjects.end());

    // Безопасное удаление объектов
    entities.erase(
        std::remove_if(entities.begin(), entities.end(), 
        [&](Entity* e) {
            bool shouldDelete = false;
            // Проверяем, помечен ли в toDelete
            if (toDelete.count(e)) {
                shouldDelete = true;
            } else {
                // Проверяем другие условия
                if (Bullet* b = dynamic_cast<Bullet*>(e)) {
                    if (b->is_off_screen()) shouldDelete = true;
                } else if (Barrier* b = dynamic_cast<Barrier*>(e)) {
                    if (b->getHealth() <= 0) shouldDelete = true;
                } else if (Explosion* exp = dynamic_cast<Explosion*>(e)) {
                    if (exp->getLifeTime() <= 0) shouldDelete = true;
                }
            }
            // Удаляем, если нужно и это не игрок
            if (shouldDelete && !e->isPlayer()) {
                delete e;
                return true;
            }
            return shouldDelete;
        }),
        entities.end()
    );

    // Проверяем поражение игрока
    checkPlayerDefeat();

    // Смена волн
    if (countEnemies() == 0 && waveActive) {
        draw();
        cout << "\033[" << SCREEN_HEIGHT/2 << ";" << SCREEN_WIDTH/2 - 5 << "H" << GREEN << currentWave << " Wave passed!" RESET << endl;
        waveActive = false;
        usleep(2000000);  // Увеличенная задержка между волнами
        startNextWave();
    }

    // Победа
    if (currentWave > 5) {
        system("clear");
        cout << "\033[" << SCREEN_HEIGHT/2 << ";" << SCREEN_WIDTH/2 - 5 << "H" << GREEN "You Win!" RESET << endl;
        cout << "\033[" << SCREEN_HEIGHT/2 + 1 << ";" << SCREEN_WIDTH/2 - 6 << "HPress any key to exit..." << endl;
        Terminal::disableRawMode();
        while (!Terminal::kbhit()) usleep(200000);
        exit(0);
    }
}

void Game::draw() {
    system("clear");

    // Отрисовка границ
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        cout << "\033[1;" << i + 1 << "H═"; // верхняя
        cout << "\033[" << SCREEN_HEIGHT << ";" << i + 1 << "H═"; // нижняя
    }

    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        cout << "\033[" << i + 1 << ";1H║"; // левая
        cout << "\033[" << i + 1 << ";" << SCREEN_WIDTH << "H║"; // правая
    }

    displayInfo();

    for (Entity* e : entities) {
        e->draw();
    }

    // Показываем сообщение о паузе
    if (isPaused) {
        cout << "\n\033[" << SCREEN_HEIGHT/2 << ";" << SCREEN_WIDTH/2 - 5 << "HPAUSED" << endl;
        cout << "\033[" << SCREEN_HEIGHT/2 + 1 << ";" << SCREEN_WIDTH/2 - 6 << "HPress any key to continue..." << endl;
    }

    cout.flush();
}

void Game::displayInfo() const {
    cout << "\033[1;1H";
    cout << "Wave: " << currentWave << " | Lives: " << player->getLives()
         << " | Score: " << score << endl;
}

void Game::spawnEnemies(int waveNumber) {
    int enemiesPerRow = 6 + waveNumber * 2;
    int rows = 0 + waveNumber;

    for (int row = 0; row < rows; ++row) {
        for (int i = 0; i < enemiesPerRow; ++i) {
            int startX = 5 + i * 4;
            int startY = 5 + row * 2;
            int type = row % 3;
            entities.push_back(new Enemy(startX, startY, 1 + waveNumber * 0.2, type));
        }
    }

    entities.push_back(new Barrier(SCREEN_WIDTH / 4, SCREEN_HEIGHT - 5));
    entities.push_back(new Barrier(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 5));
    entities.push_back(new Barrier(3 * SCREEN_WIDTH / 4, SCREEN_HEIGHT - 5));
}

int Game::countEnemies() const {
    int count = 0;
    for (Entity* e : entities) {
        if (e->isEnemy()) ++count;
    }
    return count;
}

void Game::startNextWave() {
    ++currentWave;
    spawnEnemies(currentWave);
    waveActive = true;
}

void Game::checkFleetMovement() {
    bool hitLeft = false;
    bool hitRight = false;

    for (Entity* e : entities) {
        if (e->isEnemy()) {
            if (e->getX() <= 2) hitLeft = true;
            if (e->getX() >= SCREEN_WIDTH - 2) hitRight = true;
        }
    }

    if (hitLeft || hitRight) {
        enemyDirection *= -1;

        for (Entity* e : entities) {
            if (e->isEnemy()) {
                e->moveX(enemyDirection); 
                e->moveY(1);             
            }
        }
    } else {
        for (Entity* e : entities) {
            if (e->isEnemy()) {
                e->moveX(enemyDirection); 
            }
        }
    }
}

void Game::checkPlayerDefeat() {
    for (Entity* e : entities) {
        if (e->isEnemy() && e->getY() >= SCREEN_HEIGHT - 5) {
            cout << "\n\033[" << SCREEN_HEIGHT/2 << ";" << SCREEN_WIDTH/2-5 << "HGame Over!\n";
            cout << "\033[" << SCREEN_HEIGHT/2+1 << ";" << SCREEN_WIDTH/2-6 << "HPress any key to exit...\n";
            Terminal::disableRawMode();
            while (!Terminal::kbhit()) usleep(100000);
            exit(0);
        }
    }
}

Game::~Game() {
    Terminal::disableRawMode();
    cout << "\033[?25h";

    for (Entity* e : entities) {
        delete e;
    }
}
