#include <cmath>
#include "Bomb.h"

#define FUSE_TIME 4000        // ms hasta explotar
#define EXPLOSION_TIME 1500    // ms que dura el rayo
#define PULSE_SLOW 0.010f     // rad/ms  (periodo ~630 ms)
#define PULSE_FAST 0.025f     // rad/ms  (periodo ~250 ms), en el último segundo
#define PULSE_AMOUNT 0.15f

Bomb::Bomb()
{
	sprite = NULL;
	solid = false;
	state = BOMB_FUSE;
	timer = 0;
	phase = 0.f;
}

Bomb::~Bomb()
{
	if (sprite != NULL)
		delete sprite;
}

void Bomb::init(const glm::ivec2& tileMapPos, int col, int row, ShaderProgram& shaderProgram)
{
	tileMapDispl = tileMapPos;
	posBomb = glm::ivec2(col * BOMB_TILE, row * BOMB_TILE);
	solid = false;
	state = BOMB_FUSE;
	timer = 0;
	phase = 0.f;

	spritesheet.loadFromFile("images/bomb.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(BOMB_TILE, BOMB_TILE), glm::vec2(1.f, 1.f), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 8);
	sprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	sprite->changeAnimation(0);
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posBomb.x), float(tileMapDispl.y + posBomb.y)));
}

void Bomb::update(int deltaTime)
{
	timer += deltaTime;
	if (state == BOMB_FUSE)
	{
		float speed = (timer > FUSE_TIME - 1000) ? PULSE_FAST : PULSE_SLOW;
		phase += speed * deltaTime;
		sprite->setScale(1.f + PULSE_AMOUNT * sin(phase));
	}
	else if (state == BOMB_EXPLODING && timer >= EXPLOSION_TIME)
		state = BOMB_DONE;
}

void Bomb::render()
{
	if (state == BOMB_FUSE)
		sprite->render();
}

void Bomb::explode()
{
	state = BOMB_EXPLODING;
	timer = 0;
}

bool Bomb::fuseFinished() const
{
	return state == BOMB_FUSE && timer >= FUSE_TIME;
}

bool Bomb::overlaps(const glm::ivec2& pos, const glm::ivec2& size) const
{
	return pos.x < posBomb.x + BOMB_TILE && pos.x + size.x > posBomb.x &&
		pos.y < posBomb.y + BOMB_TILE && pos.y + size.y > posBomb.y;
}