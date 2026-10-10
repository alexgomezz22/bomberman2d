#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "ShaderProgram.h"

class Bomb
{
public:
	Bomb();
	~Bomb();

	void init(const glm::ivec2& tileMapPos, const glm::ivec2& pos, ShaderProgram& shaderProgram);
	void update(int deltaTime);
	void render();

	bool overlaps(const glm::ivec2& pos, const glm::ivec2& size) const;
	glm::ivec2 getPosition() const { return posBomb; }
	bool isSolid() const { return solid; }
	void setSolid(bool s) { solid = s; }

private:
	glm::ivec2 tileMapDispl, posBomb;
	Texture spritesheet;
	Sprite* sprite;
	bool solid;
};

#endif