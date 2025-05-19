#include <iostream>
#include <vector>
#include <unordered_set>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <chrono>

using namespace std;

// ANSI-цвета
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Черный */
#define RED     "\033[31m"      /* Красный */
#define GREEN   "\033[32m"      /* Зеленый */
#define YELLOW  "\033[33m"      /* Желтый */
#define BLUE    "\033[34m"      /* Синий */
#define MAGENTA "\033[35m"      /* Фиолетовый */
#define CYAN    "\033[36m"      /* Голубой */
#define WHITE   "\033[37m"      /* Белый */

const int BULLET_SPEED = 2;
const int SCREEN_HEIGHT = 30;
const int SCREEN_WIDTH = 80;


class Terminal {
public:
    static void enableRawMode() {
        termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }

    static void disableRawMode() {
        termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag |= ICANON | ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }

    static int kbhit() {
        termios oldt, newt;
        int ch;
        int oldf;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~ICANON;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if (ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }

        return 0;
    }
};

class Entity {
protected:
    int x;
    int y;
    int speed;

public:
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual bool isEnemy() const { return false; }
    virtual bool isPlayer() const { return false; }
    virtual bool isExplosion() const { return false; }
    virtual ~Entity() {}

    int getX() const { return x; }
    int getY() const { return y; }

    // Добавленные методы
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    void moveX(int dx) { x += dx; }
    void moveY(int dy) { y += dy; }
};

class Bullet : public Entity {
private:
    int direction;
    int owner;
public:
    Bullet(int startX, int startY, int moveSpeed, int dir, int own) {
        x = startX;
        y = startY;
        speed = moveSpeed;
        direction = dir;
        owner = own;
    }

    void update() override {
        y -= speed * direction;
    }

    void draw() override {
        cout << "\033[" << y << ";" << x << "H"
             << (owner == 1 ? GREEN "*" RESET : RED "*" RESET);
    }

    bool is_off_screen() {
        return y <= 0 || y >= SCREEN_HEIGHT;
    }

    int getOwner() const { return owner; }
};

class Explosion : public Entity {
private:
    int lifeTime = 5;
public:
    Explosion(int startX, int startY) {
        x = startX;
        y = startY;
    }

    void update() override {
        lifeTime--;
    }

    void draw() override {
        if (lifeTime > 0) {
            cout << "\033[" << y << ";" << x << "H"
                 << (lifeTime % 2 == 0 ? RED "*" RESET : RED "#" RESET);
        }
    }

    bool isExplosion() const override { return true; }
    int getLifeTime() const {return lifeTime; }
};

class Player : public Entity {
private:
    int lives = 3;
    const int SHOT_COOLDOWN = 500;
    int lastShotTime =0;
public:
    Player(int startX, int startY, int moveSpeed) {
        x = startX;
        y = startY;
        speed = moveSpeed;
        lastShotTime = -SHOT_COOLDOWN;
    }

    void update() override {}
    
    void moveLeft() {
        x -= speed;
        if (x <= 2) x = 2;
    }
    void moveRight() {
        x += speed;
        if (x >= SCREEN_WIDTH - 2) x = SCREEN_WIDTH - 2;
    }

    void draw() override {
        cout << "\033[" << y << ";" << x << "H" << BLUE "=" RESET;
    }

    void shoot(std::vector<Entity*>& entities) {
        // Получаем текущее время в миллисекундах
        int currentTime = getCurrentTime();
        
        // Проверяем, прошла ли перезарядка
        if (currentTime - lastShotTime >= SHOT_COOLDOWN) {
            entities.push_back(new Bullet(x, y - 1, BULLET_SPEED, 1, 1));
            lastShotTime = currentTime;
        }
    }
    
    // Вспомогательная функция для получения текущего времени
    int getCurrentTime() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }

    bool isPlayer() const override { return true; }

    void takeDamage() {
        lives--;
        if (lives <= 0) {
            cout << "\n\033[" << SCREEN_HEIGHT/2 << ";" << SCREEN_WIDTH/2-5 << "HGame Over!\n";
            cout << "\033[" << SCREEN_HEIGHT/2+1 << ";" << SCREEN_WIDTH/2-6 << "HPress any key to exit...\n";
            Terminal::disableRawMode();
            while (!Terminal::kbhit()) usleep(100000);
            cout << "\033[?25h";  // Показать курсор
            Terminal::disableRawMode();
            while (!Terminal::kbhit()) usleep(100000);
            getchar();  // Очистить буфер
            exit(0);
        }
    }

    int getLives() const { return lives; }
};

class Enemy : public Entity {
private:
    int type;
    int health;
    int direction;
public:
    Enemy(int startX, int startY, int moveSpeed, int t) {
        x = startX;
        y = startY;
        speed = moveSpeed;
        type = t;
        direction = rand() % 2 ? 1 : -1;
        switch (type) {
            case 0: health = 1; break;
            case 1: health = 2; break;
            case 2: health = 3; break;
        }
    }

    void update() override {}

    void draw() override {
        char symbol = ' ';
        switch (type) {
            case 0: symbol = 'O'; break;
            case 1: symbol = '#'; break;
            case 2: symbol = 'W'; break;
        }
        cout << "\033[" << y << ";" << x << "H" << GREEN << symbol << RESET;
    }

    void shoot(std::vector<Entity*>& entities) {
        if (rand() % 1012 == 0) {
            entities.push_back(new Bullet(x, y + 1, BULLET_SPEED, -1, -1));
        }
    }

    void takeDamage() {
        health--;
    }

    bool isDestroyed() const {
        return health <= 0;
    }

    bool isEnemy() const override { return true; }

    bool isHit(int bulletX, int bulletY) const {
        return abs(bulletX - x) <= 1 && abs(bulletY - y) <= 1;
    }
};

class Barrier : public Entity {
private:
    int health = 3;
public:
    Barrier(int startX, int startY) {
        x = startX;
        y = startY;
    }

    void update() override {}

    void draw() override {
        if (health > 0)
            cout << "\033[" << y << ";" << x << "H"
                 << (health == 3 ? GREEN "###" RESET :
                     (health == 2 ? GREEN "## " RESET : GREEN "#  " RESET));
    }

    void takeDamage() {
        health--;
    }

    int getHealth() const { return health; }

    bool isEnemy() const override { return false; }
};

class Game {
private:
    vector<Entity*> entities;
    Player* player;
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
    ~Game();
};

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

int main() {
    Game game;
    game.run();

    return 0;
}