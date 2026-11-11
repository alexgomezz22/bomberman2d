#ifndef _SCREEN_INCLUDE
#define _SCREEN_INCLUDE


#include <string>
#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "ShaderProgram.h"


class Screen
{

public:
	Screen();
	~Screen();

	void init(const std::string& imagePath, ShaderProgram& program);
	void render();

private:
	Texture texture;
	Sprite* sprite;
	ShaderProgram* program;
};


#endif // _SCREEN_INCLUDE