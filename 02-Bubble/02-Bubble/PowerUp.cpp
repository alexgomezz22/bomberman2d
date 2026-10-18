#include "PowerUp.h"

PowerUp::PowerUp() {
    sprite = nullptr;
}

PowerUp::~PowerUp() {
    if (sprite != nullptr) {
        delete sprite;
    }
}

void PowerUp::init(const glm::vec2& tileCoords, int tileSize, PowerUpType pType, ShaderProgram& program) {
    type = pType;
    size = glm::vec2(tileSize, tileSize);

    // Convertimos la posición de la cuadrícula a píxeles en el mundo
    position = glm::vec2((tileCoords.x + 2) * tileSize, (tileCoords.y + 1) * tileSize);

    // 1. Cargar la imagen de los powerups
    texture.loadFromFile("images/powerups.png", TEXTURE_PIXEL_FORMAT_RGBA);

    // 2. Crear el sprite
    // Tamaño en pantalla = size. Tamaño relativo UV = 0.25f (1/4 de la imagen de ancho) y 1.0f de alto
    sprite = Sprite::createSprite(size, glm::vec2(0.25f, 1.0f), &texture, &program);

    // 3. Fijamos 4 animaciones exactas (una por cada tipo de PowerUp)
    sprite->setNumberAnimations(4);

    // BOMB_UP (Columna 0 en el spritesheet)
    sprite->setAnimationSpeed(BOMB_UP, 8);
    sprite->addKeyframe(BOMB_UP, glm::vec2(0.0f, 0.0f));

    // FIRE_UP (Columna 1 en el spritesheet)
    sprite->setAnimationSpeed(FIRE_UP, 8);
    sprite->addKeyframe(FIRE_UP, glm::vec2(0.25f, 0.0f));

    // TIMER (Columna 2 en el spritesheet)
    sprite->setAnimationSpeed(TIMER, 8);
    sprite->addKeyframe(TIMER, glm::vec2(0.50f, 0.0f));

    // HEART (Columna 3 en el spritesheet)
    sprite->setAnimationSpeed(HEART, 8);
    sprite->addKeyframe(HEART, glm::vec2(0.75f, 0.0f));

    // Activamos la animación según el tipo seleccionado
    sprite->changeAnimation(type);

    // Colocamos el sprite en sus coordenadas del mundo
    sprite->setPosition(position);
}

void PowerUp::update(int deltaTime) {
    if (sprite != nullptr) {
        sprite->update(deltaTime);
    }
}

void PowerUp::render() {
    if (sprite != nullptr) {
        sprite->render();
    }
}

bool PowerUp::checkCollision(const glm::vec2& playerPos, const glm::vec2& playerSize) const {
    // Solapamiento en el eje X
    bool collisionX = (position.x + size.x > playerPos.x) &&
        (playerPos.x + playerSize.x > position.x);

    // Solapamiento en el eje Y
    bool collisionY = (position.y + size.y > playerPos.y) &&
        (playerPos.y + playerSize.y > position.y);

    // Hay colisión si se solapan en AMBOS ejes a la vez
    return collisionX && collisionY;
}