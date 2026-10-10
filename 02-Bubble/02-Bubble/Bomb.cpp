#include "Bomb.h"

#define BOMB_SIZE 16

Bomb::Bomb()
{
	sprite = NULL;
	solid = false;
}

Bomb::~Bomb()
{
	if (sprite != NULL)
		delete sprite;
}

void Bomb::init(const glm::ivec2& tileMapPos, const glm::ivec2& pos, ShaderProgram& shaderProgram)
{
	tileMapDispl = tileMapPos;
	posBomb = pos;
	solid = false;

	spritesheet.loadFromFile("images/bomb.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(BOMB_SIZE, BOMB_SIZE), glm::vec2(1.f, 1.f), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 8);
	sprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	sprite->changeAnimation(0);

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posBomb.x), float(tileMapDispl.y + posBomb.y)));
}

void Bomb::update(int deltaTime)
{
	sprite->update(deltaTime);
}

void Bomb::render()
{
	sprite->render();
}

bool Bomb::overlaps(const glm::ivec2& pos, const glm::ivec2& size) const
{
	return pos.x < posBomb.x + BOMB_SIZE && pos.x + size.x > posBomb.x &&
		pos.y < posBomb.y + BOMB_SIZE && pos.y + size.y > posBomb.y;
}