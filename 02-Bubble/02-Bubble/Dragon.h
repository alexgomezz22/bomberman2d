#ifndef DRAGON_H
#define DRAGON_H

#include "Sprite.h"
#include "TileMap.h"
#include "BombManager.h"
#include <vector>


struct Fireball {
    glm::vec2 pos;
    glm::vec2 dir;
    Sprite* sprite;
    bool active;
    float lifetime;
};

class Dragon {
public:
    Dragon();
    ~Dragon();

    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
    void update(int deltaTime, glm::vec2 playerPos);
    void render();

    void setTileMap(TileMap* tileMap) { map = tileMap; }
    void setBombManager(BombManager* bm) { bombManager = bm; }
    void setPosition(const glm::vec2& pos);

    bool isAlive() const { return hp > 0; }
    bool isDead() const { return hp <= 0; }
    void kill() { hp = 0; }
    int getHP() const { return hp; }

    glm::ivec2 getHitPos() const;
    glm::ivec2 getHitSize() const;
    std::vector<Fireball>& getFireballs() { return fireballs; }
    bool isDying() const { return deathTimer > 0.f; }

private:
    void shootFireball();

    glm::ivec2 tileMapDispl;
    glm::vec2 posDragon;
    int hp;
    int invulnerableTimer;
    int shootTimer;
    int currentDir; // 1 derecha, -1 izquierda

    Texture spritesheet;
    Sprite* sprite;

    Texture fireballTex;
    ShaderProgram* shader; // Guardado para generar fireballs

    TileMap* map;
    BombManager* bombManager;
    std::vector<Fireball> fireballs;
    glm::vec2 wanderDir;   // Dirección actual de vuelo aleatorio
    int wanderTimer;
    float deathTimer;
    
};

#endif