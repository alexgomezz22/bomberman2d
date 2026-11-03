#ifndef _HUD_INCLUDE
#define _HUD_INCLUDE

#include "Texture.h"
#include "Sprite.h"

#define HUD_MAX_LIVES 5

class Hud
{
public:
	Hud();
	~Hud();

	void init(ShaderProgram& program);
	void render(int lives, int enemies, int maxBombs, int range, int seconds);

private:
	Sprite* makeCell(ShaderProgram& program, int nAnims);

	Texture sheet, frameTex;
	Sprite* frame;
	Sprite* hearts[HUD_MAX_LIVES];
	Sprite* enemyIcon;
	Sprite* enemyTens;
	Sprite* enemyUnits;
	Sprite* bombIcon;
	Sprite* bombDigit;

	Sprite* timeMin;
	Sprite* timeSecTens;
	Sprite* timeSecUnits;
	Sprite* colon;

	Sprite* flameIcon;
	Sprite* rangeDigit;

};

#endif