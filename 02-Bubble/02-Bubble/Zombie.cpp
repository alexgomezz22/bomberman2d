#include <iostream>
#include "Zombie.h"

#define ZOMBIE_SPEED 1
#define ZOMBIE_FALL_STEP 4
#define ZOMBIE_ANIM_SPEED 6
#define ZOMBIE_FRAME 16          // tamaño de un frame en la hoja
#define ZOMBIE_SIZE 32           // tamaño en pantalla (igual que Steve)
#define ZOMBIE_MOVE_EVERY 2      // se mueve 1 px cada 2 frames
#define HIT_MARGIN_X 4
#define HIT_MARGIN_Y 2

#define ZOMBIE_DEATH_SPEED 6
#define ZOMBIE_DEATH_FRAMES 4 
#define ZOMBIE_DEATH_MS_PER_FRAME (1000 / ZOMBIE_DEATH_SPEED)
#define ZOMBIE_DEATH_LAST_FRAME_TIME ((ZOMBIE_DEATH_FRAMES - 1) * ZOMBIE_DEATH_MS_PER_FRAME)
#define ZOMBIE_DEATH_TOTAL_TIME (ZOMBIE_DEATH_FRAMES * ZOMBIE_DEATH_MS_PER_FRAME + 300)

#define SHEET_W 256.f
#define SHEET_H 512.f

enum ZombieAnims { WALK_LEFT, WALK_RIGHT, DIE_LEFT, DIE_RIGHT };

Zombie::Zombie()
{
	sprite = NULL;
	map = NULL;
	bombManager = NULL;
	dir = -1;
	dead = false;
	dying = false;
	dyingTimer = 0;
}

Zombie::~Zombie()
{
	if (sprite != NULL)
		delete sprite;
}

void Zombie::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
	tileMapDispl = tileMapPos;
	dead = false;
	dir = -1;
	dying = false;
	dyingTimer = 0;
	moveTick = 0;

	spritesheet.loadFromFile("images/enemigos_spriteheet.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(ZOMBIE_SIZE, ZOMBIE_SIZE),
		glm::vec2(ZOMBIE_FRAME / SHEET_W, ZOMBIE_FRAME / SHEET_H), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(4);

	sprite->setAnimationSpeed(WALK_RIGHT, ZOMBIE_ANIM_SPEED);
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(0 / SHEET_W, 128 / SHEET_H));
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(16 / SHEET_W, 128 / SHEET_H));
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(32 / SHEET_W, 128 / SHEET_H));
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(48 / SHEET_W, 128 / SHEET_H));

	sprite->setAnimationSpeed(WALK_LEFT, ZOMBIE_ANIM_SPEED);
	sprite->addKeyframe(WALK_LEFT, glm::vec2(0 / SHEET_W, 176 / SHEET_H));
	sprite->addKeyframe(WALK_LEFT, glm::vec2(16 / SHEET_W, 176 / SHEET_H));
	sprite->addKeyframe(WALK_LEFT, glm::vec2(32 / SHEET_W, 176 / SHEET_H));
	sprite->addKeyframe(WALK_LEFT, glm::vec2(48 / SHEET_W, 176 / SHEET_H));

	sprite->setAnimationSpeed(DIE_RIGHT, ZOMBIE_DEATH_SPEED);
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(0 / SHEET_W, 160 / SHEET_H));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(16 / SHEET_W, 160 / SHEET_H));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(32 / SHEET_W, 160 / SHEET_H));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(48 / SHEET_W, 160 / SHEET_H));

	sprite->setAnimationSpeed(DIE_LEFT, ZOMBIE_DEATH_SPEED);
	sprite->addKeyframe(DIE_LEFT, glm::vec2(0 / SHEET_W, 208 / SHEET_H));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(16 / SHEET_W, 208 / SHEET_H));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(32 / SHEET_W, 208 / SHEET_H));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(48 / SHEET_W, 208 / SHEET_H));

	sprite->changeAnimation(WALK_LEFT);

	sprite->changeAnimation(WALK_LEFT);
}

void Zombie::update(int deltaTime)
{
	if (dead)
		return;

	// Muriendo: animación una sola vez, se queda en el último frame
	if (dying)
	{
		if (dyingTimer < ZOMBIE_DEATH_LAST_FRAME_TIME)
			sprite->update(deltaTime);
		dyingTimer += deltaTime;
		if (dyingTimer >= ZOMBIE_DEATH_TOTAL_TIME)
			dead = true;
		return;
	}

	sprite->update(deltaTime);
	glm::ivec2 size(ZOMBIE_SIZE, ZOMBIE_SIZE);

	// Gravedad (solo para asentarse al aparecer)
	posZombie.y += ZOMBIE_FALL_STEP;
	bool onGround = map->collisionMoveDown(posZombie, size, &posZombie.y);

	// Movimiento horizontal
	moveTick++;
	if (onGround && moveTick >= ZOMBIE_MOVE_EVERY)
	{
		moveTick = 0;
		posZombie.x += dir * ZOMBIE_SPEED;

		bool wall = (dir < 0) ? map->collisionMoveLeft(posZombie, size)
			: map->collisionMoveRight(posZombie, size);

		int ts = map->getTileSize();
		int footX = (dir < 0) ? posZombie.x : posZombie.x + ZOMBIE_SIZE - 1;
		bool ground = map->isSolidTile(footX / ts, (posZombie.y + ZOMBIE_SIZE) / ts);

		if (wall || !ground)
		{
			posZombie.x -= dir * ZOMBIE_SPEED;
			dir = -dir;
			sprite->changeAnimation(dir < 0 ? WALK_LEFT : WALK_RIGHT);
		}
	}

	// Muerte por explosión
	if (bombManager->isInExplosion(getHitPos(), getHitSize()))
	{
		dying = true;
		dyingTimer = 0;
		sprite->changeAnimation(dir < 0 ? DIE_LEFT : DIE_RIGHT);
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posZombie.x), float(tileMapDispl.y + posZombie.y)));
}

void Zombie::render()
{
	if (!dead)
		sprite->render();
}

void Zombie::setPosition(const glm::vec2& pos)
{
	posZombie = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posZombie.x), float(tileMapDispl.y + posZombie.y)));
}

glm::ivec2 Zombie::getHitPos() const
{
	return glm::ivec2(posZombie.x + HIT_MARGIN_X, posZombie.y + HIT_MARGIN_Y);
}

glm::ivec2 Zombie::getHitSize() const
{
	return glm::ivec2(ZOMBIE_SIZE - 2 * HIT_MARGIN_X, ZOMBIE_SIZE - HIT_MARGIN_Y);
}