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

private:
	glm::ivec2 tileMapDispl, posBomb;
	Texture spritesheet;
	Sprite* sprite;
};

#endif