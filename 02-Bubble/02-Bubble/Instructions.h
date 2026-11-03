#ifndef _INSTRUCTIONS_INCLUDE
#define _INSTRUCTIONS_INCLUDE


#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "ShaderProgram.h"


class Instructions
{

public:
	Instructions();
	~Instructions();

	void init(ShaderProgram& program);
	void update(bool left, bool right);
	void render();
	void reset();

private:
	static const int NUM_PAGES = 3;

	Texture pageTexture[NUM_PAGES];
	Sprite* pageSprite[NUM_PAGES];
	ShaderProgram* program;
	int page;
};


#endif // _INSTRUCTIONS_INCLUDE