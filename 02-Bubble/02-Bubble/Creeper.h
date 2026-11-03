#ifndef _CREEPER_INCLUDE
#define _CREEPER_INCLUDE

#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "TileMap.h"
#include "BombManager.h"
#include "Player.h"

class Creeper
{
public:
	Creeper();
	~Creeper();

	void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
	void update(int deltaTime);
	void render();

	void setTileMap(TileMap* tileMap) { map = tileMap; }
	void setBombManager(BombManager* m) { bombManager = m; }
	void setPlayer(Player* p) { player = p; }
	void setPosition(const glm::vec2& pos);

	void kill();
	bool isDead() const { return dead; }
	bool isAlive() const { return !dying && !dead; }
	glm::ivec2 getHitPos() const;
	glm::ivec2 getHitSize() const;
	void respawn();

private:
	glm::ivec2 tileMapDispl, posCreeper;
	Texture spritesheet;
	Sprite* sprite;
	TileMap* map;
	BombManager* bombManager;
	Player* player;
	int dir;      // -1 izquierda, +1 derecha
	bool dead;
	bool dying;
	int dyingTimer;
	glm::vec2 spawnPos;
	int moveTick;//para que no se mueva tan rapido
};

#endif