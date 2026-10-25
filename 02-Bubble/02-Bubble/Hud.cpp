#include "Hud.h"

#define HUD_W 640.f              // debe coincidir con SCREEN_WIDTH
#define HUD_H 512.f              // y con SCREEN_HEIGHT
#define HUD_SHEET_W 256.f
#define HUD_SHEET_H 64.f
#define HUD_CELL 16.f

#define HUD_Y 480                // altura de los iconos en la barra de abajo

enum HeartAnims { HEART_FULL, HEART_EMPTY };

Hud::Hud()
{
	frame = NULL; enemyIcon = NULL; enemyTens = NULL; enemyUnits = NULL;
	bombIcon = NULL; bombDigit = NULL;
	for (int i = 0; i < HUD_MAX_LIVES; i++) hearts[i] = NULL;

	timeMin = timeSecTens = timeSecUnits = colon = NULL;
}

Hud::~Hud()
{
	delete frame; delete enemyIcon; delete enemyTens; delete enemyUnits;
	delete bombIcon; delete bombDigit;
	for (int i = 0; i < HUD_MAX_LIVES; i++) delete hearts[i];

	delete timeMin;
	delete timeSecTens;
	delete timeSecUnits;
	delete colon;
}

// Sprite de 16x16 de la hoja hud.png con nAnims animaciones (cada una con 1 frame)
Sprite* Hud::makeCell(ShaderProgram& program, int nAnims)
{
	Sprite* s = Sprite::createSprite(glm::ivec2(16, 16),
		glm::vec2(HUD_CELL / HUD_SHEET_W, HUD_CELL / HUD_SHEET_H), &sheet, &program);
	s->setNumberAnimations(nAnims);
	return s;
}

void Hud::init(ShaderProgram& program)
{
	// Marco
	frameTex.loadFromFile("images/marco.png", TEXTURE_PIXEL_FORMAT_RGBA);
	frameTex.setMagFilter(GL_NEAREST);
	frameTex.setMinFilter(GL_NEAREST);
	frame = Sprite::createSprite(glm::ivec2(int(HUD_W), int(HUD_H)), glm::vec2(1.f, 1.f), &frameTex, &program);
	frame->setNumberAnimations(1);
	frame->setAnimationSpeed(0, 8);
	frame->addKeyframe(0, glm::vec2(0.f, 0.f));
	frame->changeAnimation(0);
	frame->setPosition(glm::vec2(0.f, 0.f));

	// Hoja de iconos y números
	sheet.loadFromFile("images/hud.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sheet.setMagFilter(GL_NEAREST);
	sheet.setMinFilter(GL_NEAREST);

	// Corazones: animación 0 = lleno (0,0), animación 1 = vacío (16,0)
	for (int i = 0; i < HUD_MAX_LIVES; i++)
	{
		hearts[i] = makeCell(program, 2);
		hearts[i]->setAnimationSpeed(HEART_FULL, 8);
		hearts[i]->addKeyframe(HEART_FULL, glm::vec2(0 / HUD_SHEET_W, 0 / HUD_SHEET_H));
		hearts[i]->setAnimationSpeed(HEART_EMPTY, 8);
		hearts[i]->addKeyframe(HEART_EMPTY, glm::vec2(16 / HUD_SHEET_W, 0 / HUD_SHEET_H));
		hearts[i]->changeAnimation(HEART_FULL);
		hearts[i]->setPosition(glm::vec2(192 + i * 20, HUD_Y));      // 192, 212, 232
	}

	// Icono de enemigos (32,0) y de bombas (48,0)
	enemyIcon = makeCell(program, 1);
	enemyIcon->setAnimationSpeed(0, 8);
	enemyIcon->addKeyframe(0, glm::vec2(32 / HUD_SHEET_W, 0 / HUD_SHEET_H));
	enemyIcon->changeAnimation(0);
	enemyIcon->setPosition(glm::vec2(336, HUD_Y));

	bombIcon = makeCell(program, 1);
	bombIcon->setAnimationSpeed(0, 8);
	bombIcon->addKeyframe(0, glm::vec2(48 / HUD_SHEET_W, 0 / HUD_SHEET_H));
	bombIcon->changeAnimation(0);
	bombIcon->setPosition(glm::vec2(480, HUD_Y));

	// Dígitos: animación d = número d, en la fila y = 16, x = d * 16
	enemyTens = makeCell(program, 10);
	enemyUnits = makeCell(program, 10);
	bombDigit = makeCell(program, 10);
	for (int d = 0; d < 10; d++)
	{
		enemyTens->setAnimationSpeed(d, 8);
		enemyTens->addKeyframe(d, glm::vec2(d * 16 / HUD_SHEET_W, 16 / HUD_SHEET_H));
		enemyUnits->setAnimationSpeed(d, 8);
		enemyUnits->addKeyframe(d, glm::vec2(d * 16 / HUD_SHEET_W, 16 / HUD_SHEET_H));
		bombDigit->setAnimationSpeed(d, 8);
		bombDigit->addKeyframe(d, glm::vec2(d * 16 / HUD_SHEET_W, 16 / HUD_SHEET_H));
	}
	enemyTens->changeAnimation(0);
	enemyUnits->changeAnimation(0);
	bombDigit->changeAnimation(0);
	enemyTens->setPosition(glm::vec2(360, HUD_Y));
	enemyUnits->setPosition(glm::vec2(376, HUD_Y));
	bombDigit->setPosition(glm::vec2(504, HUD_Y));

	// Temporizador: minuto, ":", segundos (centrado en el hueco de x = 48..144)
	timeMin = makeCell(program, 10);
	timeSecTens = makeCell(program, 10);
	timeSecUnits = makeCell(program, 10);
	Sprite* timeSprites[3] = { timeMin, timeSecTens, timeSecUnits };
	for (int t = 0; t < 3; t++)
	{
		for (int d = 0; d < 10; d++)
		{
			timeSprites[t]->setAnimationSpeed(d, 8);
			timeSprites[t]->addKeyframe(d, glm::vec2(d * 16 / HUD_SHEET_W, 16 / HUD_SHEET_H));
		}
		timeSprites[t]->changeAnimation(0);
	}
	timeMin->setPosition(glm::vec2(64, HUD_Y));
	timeSecTens->setPosition(glm::vec2(96, HUD_Y));
	timeSecUnits->setPosition(glm::vec2(112, HUD_Y));

	colon = makeCell(program, 1);
	colon->setAnimationSpeed(0, 8);
	colon->addKeyframe(0, glm::vec2(64 / HUD_SHEET_W, 0 / HUD_SHEET_H));   // ":" en la hoja
	colon->changeAnimation(0);
	colon->setPosition(glm::vec2(80, HUD_Y));
}

void Hud::render(int lives, int enemies, int maxBombs, int seconds)
{
	if (enemies > 99) enemies = 99;
	if (enemies < 0) enemies = 0;
	if (maxBombs > 9) maxBombs = 9;
	if (maxBombs < 0) maxBombs = 0;

	frame->render();

	int shown = (lives > 3) ? lives : 3;          // siempre 3 como mínimo (los vacíos salen grises)
	if (shown > HUD_MAX_LIVES) shown = HUD_MAX_LIVES;
	for (int i = 0; i < shown; i++)
	{
		hearts[i]->changeAnimation(i < lives ? HEART_FULL : HEART_EMPTY);
		hearts[i]->render();
	}

	enemyIcon->render();
	enemyTens->changeAnimation(enemies / 10);
	enemyTens->render();
	enemyUnits->changeAnimation(enemies % 10);
	enemyUnits->render();

	bombIcon->render();
	bombDigit->changeAnimation(maxBombs);
	bombDigit->render();

	if (seconds < 0) seconds = 0;
	int minutes = seconds / 60;
	if (minutes > 9) minutes = 9;

	timeMin->changeAnimation(minutes);
	timeMin->render();
	colon->render();
	timeSecTens->changeAnimation((seconds % 60) / 10);
	timeSecTens->render();
	timeSecUnits->changeAnimation(seconds % 10);
	timeSecUnits->render();
}