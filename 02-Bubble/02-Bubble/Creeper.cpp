#include <cstdlib>
#include "Creeper.h"
#include "Audio.h"

#define CREEPER_SPEED 1            // igual que el enderman y el zombie
#define CREEPER_MOVE_EVERY 2       // se mueve 1 px cada 2 frames
#define CREEPER_FALL_STEP 4
#define CREEPER_ANIM_SPEED 6
#define CREEPER_FRAME 16           // tamaño de un frame en la hoja
#define CREEPER_SIZE 32            // tamaño en pantalla (igual que Steve)
#define CREEPER_SENSE_TILES 3      // a cuántos tiles detecta a Steve
#define HIT_MARGIN_X 4
#define HIT_MARGIN_Y 2

#define CREEPER_DEATH_SPEED 6
#define CREEPER_DEATH_FRAMES 4
#define CREEPER_DEATH_MS_PER_FRAME (1000 / CREEPER_DEATH_SPEED)
#define CREEPER_DEATH_LAST_FRAME_TIME ((CREEPER_DEATH_FRAMES - 1) * CREEPER_DEATH_MS_PER_FRAME)
#define CREEPER_DEATH_TOTAL_TIME (CREEPER_DEATH_FRAMES * CREEPER_DEATH_MS_PER_FRAME + 300)

#define SHEET_W 256.f
#define SHEET_H 512.f

enum CreeperAnims { C_WALK_LEFT, C_WALK_RIGHT, C_DIE_LEFT, C_DIE_RIGHT };

Creeper::Creeper()
{
	sprite = NULL;
	map = NULL;
	bombManager = NULL;
	player = NULL;
	dir = -1;
	dead = false;
	dying = false;
	dyingTimer = 0;
	moveTick = 0;
}

Creeper::~Creeper()
{
	if (sprite != NULL)
		delete sprite;
}

void Creeper::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
	tileMapDispl = tileMapPos;
	dead = false;
	dir = -1;
	dying = false;
	dyingTimer = 0;
	moveTick = 0;

	spritesheet.loadFromFile("images/enemigos_spriteheet.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(CREEPER_SIZE, CREEPER_SIZE),
		glm::vec2(CREEPER_FRAME / SHEET_W, CREEPER_FRAME / SHEET_H), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(4);

	sprite->setAnimationSpeed(C_WALK_RIGHT, CREEPER_ANIM_SPEED);
	sprite->addKeyframe(C_WALK_RIGHT, glm::vec2(0 / SHEET_W, 16 / SHEET_H));
	sprite->addKeyframe(C_WALK_RIGHT, glm::vec2(16 / SHEET_W, 16 / SHEET_H));
	sprite->addKeyframe(C_WALK_RIGHT, glm::vec2(32 / SHEET_W, 16 / SHEET_H));
	sprite->addKeyframe(C_WALK_RIGHT, glm::vec2(48 / SHEET_W, 16 / SHEET_H));

	sprite->setAnimationSpeed(C_WALK_LEFT, CREEPER_ANIM_SPEED);
	sprite->addKeyframe(C_WALK_LEFT, glm::vec2(0 / SHEET_W, 64 / SHEET_H));
	sprite->addKeyframe(C_WALK_LEFT, glm::vec2(16 / SHEET_W, 64 / SHEET_H));
	sprite->addKeyframe(C_WALK_LEFT, glm::vec2(32 / SHEET_W, 64 / SHEET_H));
	sprite->addKeyframe(C_WALK_LEFT, glm::vec2(48 / SHEET_W, 64 / SHEET_H));

	sprite->setAnimationSpeed(C_DIE_RIGHT, CREEPER_DEATH_SPEED);
	sprite->addKeyframe(C_DIE_RIGHT, glm::vec2(0 / SHEET_W, 48 / SHEET_H));
	sprite->addKeyframe(C_DIE_RIGHT, glm::vec2(16 / SHEET_W, 48 / SHEET_H));
	sprite->addKeyframe(C_DIE_RIGHT, glm::vec2(32 / SHEET_W, 48 / SHEET_H));
	sprite->addKeyframe(C_DIE_RIGHT, glm::vec2(48 / SHEET_W, 48 / SHEET_H));

	sprite->setAnimationSpeed(C_DIE_LEFT, CREEPER_DEATH_SPEED);
	sprite->addKeyframe(C_DIE_LEFT, glm::vec2(0 / SHEET_W, 96 / SHEET_H));
	sprite->addKeyframe(C_DIE_LEFT, glm::vec2(16 / SHEET_W, 96 / SHEET_H));
	sprite->addKeyframe(C_DIE_LEFT, glm::vec2(32 / SHEET_W, 96 / SHEET_H));
	sprite->addKeyframe(C_DIE_LEFT, glm::vec2(48 / SHEET_W, 96 / SHEET_H));

	sprite->changeAnimation(C_WALK_LEFT);
}

void Creeper::update(int deltaTime)
{
	if (dead)
		return;

	// Muriendo: animación una sola vez, se queda en el último frame
	if (dying)
	{
		if (dyingTimer < CREEPER_DEATH_LAST_FRAME_TIME)
			sprite->update(deltaTime);
		dyingTimer += deltaTime;
		if (dyingTimer >= CREEPER_DEATH_TOTAL_TIME)
			dead = true;
		return;
	}

	sprite->update(deltaTime);
	glm::ivec2 size(CREEPER_SIZE, CREEPER_SIZE);

	// Gravedad (solo para asentarse al aparecer)
	posCreeper.y += CREEPER_FALL_STEP;
	bool onGround = map->collisionMoveDown(posCreeper, size, &posCreeper.y);

	if (onGround)
	{
		int ts = map->getTileSize();

		// ¿Steve está cerca (2 tiles a izquierda o derecha) y en su mismo suelo?
		bool chasing = false;
		int dx = 0;
		if (player != NULL && !player->isDead())
		{
			glm::ivec2 pp = player->getHitPos(), ps = player->getHitSize();
			dx = (pp.x + ps.x / 2) - (posCreeper.x + CREEPER_SIZE / 2);                 // + = Steve a la derecha
			int dy = (pp.y + ps.y) - (posCreeper.y + CREEPER_SIZE);                      // diferencia de altura de los pies
			if (abs(dx) <= CREEPER_SENSE_TILES * ts && abs(dy) < ts)
			{
				chasing = true;
				int newDir = (dx < 0) ? -1 : 1;
				if (newDir != dir)
				{
					dir = newDir;
					sprite->changeAnimation(dir < 0 ? C_WALK_LEFT : C_WALK_RIGHT);
				}
			}
		}

		// Si ya está justo encima de Steve, no se mueve (evita que tiemble)
		moveTick++;
		if (moveTick >= CREEPER_MOVE_EVERY && !(chasing && abs(dx) <= CREEPER_SPEED))
		{
			moveTick = 0;
			posCreeper.x += dir * CREEPER_SPEED;

			bool wall = (dir < 0) ? map->collisionMoveLeft(posCreeper, size)
				: map->collisionMoveRight(posCreeper, size);

			int footX = (dir < 0) ? posCreeper.x : posCreeper.x + CREEPER_SIZE - 1;
			bool ground = map->isSolidTile(footX / ts, (posCreeper.y + CREEPER_SIZE) / ts);

			if (wall || !ground)
			{
				posCreeper.x -= dir * CREEPER_SPEED;
				if (!chasing)                       // patrullando gira; persiguiendo se queda parado
				{
					dir = -dir;
					sprite->changeAnimation(dir < 0 ? C_WALK_LEFT : C_WALK_RIGHT);
				}
			}
		}
	}

	// Muerte por explosión
	if (bombManager->isInExplosion(getHitPos(), getHitSize())) {
		Audio::instance().play(Audio::CREEPER_DEATH);
		kill();
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posCreeper.x), float(tileMapDispl.y + posCreeper.y)));
}

void Creeper::kill()
{
	if (!isAlive())
		return;
	dying = true;
	dyingTimer = 0;
	sprite->changeAnimation(dir < 0 ? C_DIE_LEFT : C_DIE_RIGHT);
}

void Creeper::render()
{
	if (!dead)
		sprite->render();
}

void Creeper::setPosition(const glm::vec2& pos)
{
	posCreeper = pos;
	spawnPos = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posCreeper.x), float(tileMapDispl.y + posCreeper.y)));
}

glm::ivec2 Creeper::getHitPos() const
{
	return glm::ivec2(posCreeper.x + HIT_MARGIN_X, posCreeper.y + HIT_MARGIN_Y);
}

glm::ivec2 Creeper::getHitSize() const
{
	return glm::ivec2(CREEPER_SIZE - 2 * HIT_MARGIN_X, CREEPER_SIZE - HIT_MARGIN_Y);
}

void Creeper::respawn()
{
	dead = false;
	dying = false;
	dyingTimer = 0;
	moveTick = 0;
	dir = -1;
	sprite->changeAnimation(C_WALK_LEFT);
	setPosition(spawnPos);
}