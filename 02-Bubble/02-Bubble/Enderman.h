#ifndef _ENDERMAN_INCLUDE
#define _ENDERMAN_INCLUDE

#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "TileMap.h"
#include "BombManager.h"

class Enderman
{
public:
	Enderman();
	~Enderman();

	void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
	void update(int deltaTime);
	void render();

	void setTileMap(TileMap* tileMap) { map = tileMap; }
	void setBombManager(BombManager* m) { bombManager = m; }
	void setPosition(const glm::vec2& pos);

	bool isDead() const { return dead; }
	bool isAlive() const { return !dying && !dead; }
	glm::ivec2 getHitPos() const;
	glm::ivec2 getHitSize() const;
	void respawn();

private:
	bool canStandAt(int x, int y);

	glm::ivec2 tileMapDispl, posEnder;
	Texture spritesheet;
	Sprite* sprite;
	TileMap* map;
	BombManager* bombManager;
	int dir;              // -1 izquierda, +1 derecha
	bool dead, dying;
	int dyingTimer, moveTick, teleportTimer;
	glm::vec2 spawnPos;
};

#endif