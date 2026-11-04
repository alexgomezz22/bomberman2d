#include "Dragon.h"
#include <cmath>

#define DRAGON_SIZE 128 
#define FIREBALL_SIZE 16
#define DRAGON_SPEED 0.06f
#define FIREBALL_SPEED 0.15f

enum DragonAnims { FLY_LEFT, FLY_RIGHT, HIT_LEFT, HIT_RIGHT };

Dragon::Dragon() {
    sprite = NULL; map = NULL; bombManager = NULL;
    hp = 5; invulnerableTimer = 0; shootTimer = 1500; currentDir = -1;
    wanderTimer = 0;
    wanderDir = glm::vec2(-1.f, 0.f);
}

Dragon::~Dragon() {
    if (sprite != NULL) delete sprite;
    for (auto& fb : fireballs) if (fb.sprite != NULL) delete fb.sprite;
}

void Dragon::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) {
    tileMapDispl = tileMapPos;
    shader = &shaderProgram;
    hp = 5;
    invulnerableTimer = 0;

    spritesheet.loadFromFile("images/drac.png", TEXTURE_PIXEL_FORMAT_RGBA);
    
    sprite = Sprite::createSprite(glm::ivec2(DRAGON_SIZE, DRAGON_SIZE), glm::vec2(64.f / 256.f, 64.f / 256.f), &spritesheet, shader);
    sprite->setNumberAnimations(4);

    sprite->setAnimationSpeed(FLY_RIGHT, 8);
    sprite->addKeyframe(FLY_RIGHT, glm::vec2(0.f, 0.f));
    sprite->addKeyframe(FLY_RIGHT, glm::vec2(0.25f, 0.f));
    sprite->addKeyframe(FLY_RIGHT, glm::vec2(0.50f, 0.f));

    sprite->setAnimationSpeed(FLY_LEFT, 8);
    sprite->addKeyframe(FLY_LEFT, glm::vec2(0.f, 0.25f));
    sprite->addKeyframe(FLY_LEFT, glm::vec2(0.25f, 0.25f));
    sprite->addKeyframe(FLY_LEFT, glm::vec2(0.50f, 0.25f));

    sprite->setAnimationSpeed(HIT_RIGHT, 8);
    sprite->addKeyframe(HIT_RIGHT, glm::vec2(0.f, 0.50f));

    sprite->setAnimationSpeed(HIT_LEFT, 8);
    sprite->addKeyframe(HIT_LEFT, glm::vec2(0.f, 0.75f));

    sprite->changeAnimation(FLY_LEFT);
    fireballTex.loadFromFile("images/Dragon_Fireball.png", TEXTURE_PIXEL_FORMAT_RGBA);
}

void Dragon::update(int deltaTime, glm::vec2 playerPos) {
    if (hp <= 0) return;

    // 1. Recibir daño de bombas
    if (invulnerableTimer > 0) {
        invulnerableTimer -= deltaTime;
        if (invulnerableTimer <= 0) {
            sprite->changeAnimation(currentDir == 1 ? FLY_RIGHT : FLY_LEFT);
        }
    }
    else if (bombManager->isInExplosion(getHitPos(), getHitSize())) {
        hp--;
        invulnerableTimer = 1500;
        sprite->changeAnimation(currentDir == 1 ? HIT_RIGHT : HIT_LEFT);
    }

    // 2. Movimiento libre aleatorio del dragón
    wanderTimer -= deltaTime;
    if (wanderTimer <= 0) {
        wanderTimer = 1500 + (rand() % 1500);
        float angle = (rand() % 360) * 3.14159f / 180.f;
        wanderDir = glm::vec2(cos(angle), sin(angle));
    }

    posDragon += wanderDir * DRAGON_SPEED * float(deltaTime);

    int mapW = map->getMapSize().x * map->getTileSize();
    int mapH = map->getMapSize().y * map->getTileSize();

    if (posDragon.x < 16) { posDragon.x = 16; wanderDir.x = -wanderDir.x; }
    if (posDragon.x > mapW - DRAGON_SIZE - 16) { posDragon.x = mapW - DRAGON_SIZE - 16; wanderDir.x = -wanderDir.x; }
    if (posDragon.y < 16) { posDragon.y = 16; wanderDir.y = -wanderDir.y; }
    if (posDragon.y > mapH - DRAGON_SIZE - 16) { posDragon.y = mapH - DRAGON_SIZE - 16; wanderDir.y = -wanderDir.y; }

    if (invulnerableTimer <= 0) {
        if (wanderDir.x > 0 && currentDir != 1) { currentDir = 1; sprite->changeAnimation(FLY_RIGHT); }
        else if (wanderDir.x < 0 && currentDir != -1) { currentDir = -1; sprite->changeAnimation(FLY_LEFT); }
    }
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posDragon.x), float(tileMapDispl.y + posDragon.y)));

    // 3. Disparar bola de fuego en dirección aleatoria
    shootTimer -= deltaTime;
    if (shootTimer <= 0 && invulnerableTimer <= 0) {
        shootTimer = 5000; // Frecuencia de disparo
        shootFireball();
    }

    // 4. Actualizar movimiento, rebotes y tiempo de vida de las bolas de fuego
    float minX = 16.f;
    float maxX = float(mapW - FIREBALL_SIZE - 16);
    float minY = 16.f;
    float maxY = float(mapH - FIREBALL_SIZE - 16);

    for (auto& fb : fireballs) {
        if (fb.active) {
            // Movimiento
            fb.pos += fb.dir * FIREBALL_SPEED * float(deltaTime);

            // Rebotes en los bordes del mapa/pantalla
            if (fb.pos.x <= minX) { fb.pos.x = minX; fb.dir.x = -fb.dir.x; }
            if (fb.pos.x >= maxX) { fb.pos.x = maxX; fb.dir.x = -fb.dir.x; }
            if (fb.pos.y <= minY) { fb.pos.y = minY; fb.dir.y = -fb.dir.y; }
            if (fb.pos.y >= maxY) { fb.pos.y = maxY; fb.dir.y = -fb.dir.y; }

            fb.sprite->setPosition(glm::vec2(tileMapDispl.x + fb.pos.x, tileMapDispl.y + fb.pos.y));

            // Descontar tiempo de vida (5000ms)
            fb.lifetime -= deltaTime;
            if (fb.lifetime <= 0) {
                fb.active = false;
            }
        }
    }
}

void Dragon::shootFireball() {
    glm::vec2 fbPos = posDragon + glm::vec2(DRAGON_SIZE / 2.f - FIREBALL_SIZE / 2.f, DRAGON_SIZE / 2.f - FIREBALL_SIZE / 2.f);

    // Ángulo aleatorio (0 a 360 grados)
    float angle = (rand() % 360) * 3.14159f / 180.f;
    glm::vec2 dir = glm::vec2(cos(angle), sin(angle));

    Fireball fb;
    fb.pos = fbPos;
    fb.dir = dir;
    fb.active = true;
    fb.lifetime = 5000.f; // 5 segundos de duración
    fb.sprite = Sprite::createSprite(glm::ivec2(FIREBALL_SIZE, FIREBALL_SIZE), glm::vec2(1.f, 1.f), &fireballTex, shader);
    fb.sprite->setPosition(glm::vec2(tileMapDispl.x + fb.pos.x, tileMapDispl.y + fb.pos.y));
    fireballs.push_back(fb);
}

void Dragon::render() {
    for (auto& fb : fireballs) {
        if (fb.active) {
            // Transparencia normal = 1.0. En el último segundo (1000ms), baja linealmente a 0.0
            float alpha = 1.0f;
            if (fb.lifetime < 1000.f) {
                alpha = fb.lifetime / 1000.f;
                if (alpha < 0.f) alpha = 0.f;
            }

            // Aplicar alpha al shader antes de dibujar la bola
            shader->setUniform4f("color", 1.0f, 1.0f, 1.0f, alpha);
            fb.sprite->render();
            // Restaurar la opacidad completa para el resto del render
            shader->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
    if (hp > 0) sprite->render();
}

void Dragon::setPosition(const glm::vec2& pos) { posDragon = pos; }
glm::ivec2 Dragon::getHitPos() const { return glm::ivec2(posDragon.x + 16, posDragon.y + 16); }
glm::ivec2 Dragon::getHitSize() const { return glm::ivec2(DRAGON_SIZE - 32, DRAGON_SIZE - 32); }