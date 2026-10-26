#ifndef _POWERUP_INCLUDE
#define _POWERUP_INCLUDE

#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "ShaderProgram.h"
#include <vector>

enum PowerUpType { BOMB_UP, FIRE_UP, TIMER, LIFE_UP};

class PowerUp
{
private:
    glm::vec2 tilePos;      // Posición en la rejilla (x, y)
    glm::vec2 position;     // Posición real en píxeles (mundo)
    glm::vec2 size = {16, 16};
    PowerUpType type;       // Tipo: SPEED, MORE_BOMBS, FLAME_RANGE...
    Sprite* sprite;
    Texture texture;

public:
    PowerUp();
    ~PowerUp();

    void init(const glm::vec2& tileCoords, int tileSize, PowerUpType pType, ShaderProgram& program);
    void update(int deltaTime);
    void render();
    bool checkCollision(const glm::vec2& playerPos, const glm::vec2& playerSize) const;

    glm::vec2 getPosition() const { return position; }
    PowerUpType getType() const { return type; }
};

#endif // _POWERUP_INCLUDE