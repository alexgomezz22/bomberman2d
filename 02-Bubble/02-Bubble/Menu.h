#ifndef _MENU_INCLUDE
#define _MENU_INCLUDE


#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "ShaderProgram.h"


class Menu
{

public:
	enum Choice { NONE = -1, PLAY, INSTRUCTIONS, CREDITS, EXIT };

	Menu();
	~Menu();

	void init(ShaderProgram& program);
	Choice update(bool up, bool down, bool enter);
	void render();
	void reset();

private:
	static const int NUM_OPTIONS = 4;

	Texture backgroundTexture;
	Texture cursorTexture;
	Sprite* background;
	Sprite* cursor;
	ShaderProgram* program;
	int option;
};


#endif // _MENU_INCLUDE