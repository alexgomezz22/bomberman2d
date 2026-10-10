#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include <vector>
#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "ShaderProgram.h"

#define BOMB_TILE 16

enum FlameType { FLAME_CENTER, FLAME_H, FLAME_V, FLAME_UP, FLAME_DOWN, FLAME_LEFT, FLAME_RIGHT, FLAME_TYPES };
enum BombState { BOMB_FUSE, BOMB_EXPLODING, BOMB_DONE };

struct FlameCell
{
	glm::ivec2 tile;
	int type;
	int dist;
};

class Bomb
{
public:
	Bomb();
	~Bomb();

	void init(const glm::ivec2& tileMapPos, int col, int row, ShaderProgram& shaderProgram);
	void update(int deltaTime);
	void render();                      // solo dibuja la bomba (no el rayo)

	void explode();                     // pasa a EXPLODING
	bool fuseFinished() const;
	int getTimer() const { return timer; }

	bool overlaps(const glm::ivec2& pos, const glm::ivec2& size) const;
	glm::ivec2 getPosition() const { return posBomb; }
	glm::ivec2 getTile() const { return glm::ivec2(posBomb.x / BOMB_TILE, posBomb.y / BOMB_TILE); }
	BombState getState() const { return state; }
	bool isSolid() const { return solid && state == BOMB_FUSE; }
	void setSolid(bool s) { solid = s; }
	std::vector<FlameCell>& getFlames() { return flames; }

private:
	glm::ivec2 tileMapDispl, posBomb;
	Texture spritesheet;
	Sprite* sprite;
	bool solid;
	BombState state;
	int timer;
	float phase;
	std::vector<FlameCell> flames;
};

#endif