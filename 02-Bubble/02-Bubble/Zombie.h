#ifndef _ZOMBIE_INCLUDE
#define _ZOMBIE_INCLUDE

#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "TileMap.h"
#include "BombManager.h"

class Zombie
{
public:
	Zombie();
	~Zombie();

	void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
	void update(int deltaTime);
	void render();

	void setTileMap(TileMap* tileMap) { map = tileMap; }
	void setBombManager(BombManager* m) { bombManager = m; }
	void setPosition(const glm::vec2& pos);

	bool isDead() const { return dead; }
	bool isAlive() const { return !dying && !dead; }   // false en cuanto le toca el rayo
	glm::ivec2 getHitPos() const;
	glm::ivec2 getHitSize() const;

private:
	glm::ivec2 tileMapDispl, posZombie;
	Texture spritesheet;
	Sprite* sprite;
	TileMap* map;
	BombManager* bombManager;
	int dir;      // -1 izquierda +1 derecha
	bool dead;
	bool dying;
	int dyingTimer;
	int moveTick;

};

#endif