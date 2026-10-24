#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#define HIT_OFFSET_X 8
#define HIT_OFFSET_Y 4
#define HIT_W 16
#define HIT_H 28
#define DEATH_ANIM_SPEED 6
#define DEATH_FRAMES 4
#define DEATH_MS_PER_FRAME (1000 / DEATH_ANIM_SPEED)
#define DEATH_LAST_FRAME_TIME ((DEATH_FRAMES - 1) * DEATH_MS_PER_FRAME)
#define DEATH_ANIM_TIME (DEATH_FRAMES * DEATH_MS_PER_FRAME)

#include "Sprite.h"
#include "TileMap.h"
#include <vector>
#include "Bomb.h"
#include "BombManager.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{
public:
	Player();
	~Player();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	void setBombManager(BombManager* m) { bombManager = m; }
	bool isDead() const { return dead; }
	bool isDeathFinished() const { return dead && deathTimer >= DEATH_ANIM_TIME + 500; }
	void respawn(const glm::vec2& pos);
	void increaseMaxBombs() { maxBombs++; }
	int getMaxBombs() const { return maxBombs; }
	void kill() { if (!dead) die(); }
	glm::ivec2 getHitPos() const { return glm::ivec2(posPlayer.x + HIT_OFFSET_X, posPlayer.y + HIT_OFFSET_Y); }
	glm::ivec2 getHitSize() const { return glm::ivec2(HIT_W, HIT_H); }

private:
	bool bPuttingBomb, bBWasDown;
	int bombTimer;
	BombManager* bombManager;
	bool bombBlocks(const glm::ivec2& pos);
	bool bombLandDown();
	bool bJumping;
	bool facingLeft;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	bool dead;
	int deathTimer;
	void die();
	int maxBombs;
};


#endif // _PLAYER_INCLUDE


