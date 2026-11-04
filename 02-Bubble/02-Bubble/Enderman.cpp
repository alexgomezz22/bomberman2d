#include <iostream>
#include "Enderman.h"
#include "Audio.h"

#define ENDER_SPEED 1
#define ENDER_FALL_STEP 4
#define ENDER_ANIM_SPEED 6
#define ENDER_FRAME_W 16         // ancho de un frame en la hoja      TODO
#define ENDER_FRAME_H 32         // alto de un frame en la hoja       TODO
#define ENDER_W 32               // ancho en pantalla
#define ENDER_H 64               // alto en pantalla (el doble que Steve)
#define ENDER_MOVE_EVERY 2       // se mueve 1 px cada 2 frames (igual que el zombie)
#define ENDER_TELEPORT_TIME 5000 // ms entre teletransportes
#define ENDER_TELEPORT_DIST 32   // 2 tiles de 16 px
#define HIT_MARGIN_X 4
#define HIT_MARGIN_Y 2

#define ENDER_DEATH_SPEED 6
#define ENDER_DEATH_FRAMES 4
#define ENDER_DEATH_MS_PER_FRAME (1000 / ENDER_DEATH_SPEED)
#define ENDER_DEATH_LAST_FRAME_TIME ((ENDER_DEATH_FRAMES - 1) * ENDER_DEATH_MS_PER_FRAME)
#define ENDER_DEATH_TOTAL_TIME (ENDER_DEATH_FRAMES * ENDER_DEATH_MS_PER_FRAME + 300)

#define SHEET_W 256.f            // TODO
#define SHEET_H 512.f            // TODO

enum EnderAnims { WALK_LEFT, WALK_RIGHT, DIE_LEFT, DIE_RIGHT };

Enderman::Enderman()
{
	sprite = NULL;
	map = NULL;
	bombManager = NULL;
	dir = -1;
	dead = false;
	dying = false;
	dyingTimer = 0;
	moveTick = 0;
	teleportTimer = 0;
}

Enderman::~Enderman()
{
	if (sprite != NULL)
		delete sprite;
}

void Enderman::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
	tileMapDispl = tileMapPos;
	dead = false;
	dir = -1;
	dying = false;
	dyingTimer = 0;
	moveTick = 0;
	teleportTimer = 0;

	spritesheet.loadFromFile("images/enemigos_spriteheet.png", TEXTURE_PIXEL_FORMAT_RGBA);   // TODO: ruta de la hoja
	sprite = Sprite::createSprite(glm::ivec2(ENDER_W, ENDER_H),
		glm::vec2(ENDER_FRAME_W / SHEET_W, ENDER_FRAME_H / SHEET_H), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(4);

	sprite->setAnimationSpeed(WALK_RIGHT, ENDER_ANIM_SPEED);
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(0 / SHEET_W, 256 / SHEET_H));
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(16 / SHEET_W, 256 / SHEET_H));
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(32 / SHEET_W, 256 / SHEET_H));
	sprite->addKeyframe(WALK_RIGHT, glm::vec2(48 / SHEET_W, 256 / SHEET_H));

	sprite->setAnimationSpeed(WALK_LEFT, ENDER_ANIM_SPEED);
	sprite->addKeyframe(WALK_LEFT, glm::vec2(0 / SHEET_W, 352 / SHEET_H));
	sprite->addKeyframe(WALK_LEFT, glm::vec2(16 / SHEET_W, 352 / SHEET_H));
	sprite->addKeyframe(WALK_LEFT, glm::vec2(32 / SHEET_W, 352 / SHEET_H));
	sprite->addKeyframe(WALK_LEFT, glm::vec2(48 / SHEET_W, 352 / SHEET_H));

	sprite->setAnimationSpeed(DIE_RIGHT, ENDER_DEATH_SPEED);
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(0 / SHEET_W, 320 / SHEET_H));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(16 / SHEET_W, 320 / SHEET_H));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(32 / SHEET_W, 320 / SHEET_H));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(48 / SHEET_W, 320 / SHEET_H));

	sprite->setAnimationSpeed(DIE_LEFT, ENDER_DEATH_SPEED);
	sprite->addKeyframe(DIE_LEFT, glm::vec2(0 / SHEET_W, 416 / SHEET_H));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(16 / SHEET_W, 416 / SHEET_H));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(32 / SHEET_W, 416 / SHEET_H));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(48 / SHEET_W, 416 / SHEET_H));

	sprite->changeAnimation(WALK_LEFT);
}

// Cabe el enderman con la esquina superior izquierda en (x, y)?
// Sin tiles sólidos dentro y con suelo bajo los dos pies.
bool Enderman::canStandAt(int x, int y)
{
	int ts = map->getTileSize();
	int cols = int(map->getMapSize().x);
	if (x < 0 || x + ENDER_W > cols * ts)
		return false;

	int c0 = x / ts, c1 = (x + ENDER_W - 1) / ts;
	int r0 = y / ts, r1 = (y + ENDER_H - 1) / ts;
	for (int r = r0; r <= r1; r++)
		for (int c = c0; c <= c1; c++)
			if (map->isSolidTile(c, r))
				return false;

	int rf = (y + ENDER_H) / ts;     // fila justo bajo los pies
	return map->isSolidTile(c0, rf) && map->isSolidTile(c1, rf);
}

void Enderman::update(int deltaTime)
{
	if (dead)
		return;

	// Muriendo: animación una sola vez, se queda en el último frame
	if (dying)
	{
		if (dyingTimer < ENDER_DEATH_LAST_FRAME_TIME)
			sprite->update(deltaTime);
		dyingTimer += deltaTime;
		if (dyingTimer >= ENDER_DEATH_TOTAL_TIME)
			dead = true;
		return;
	}

	sprite->update(deltaTime);
	glm::ivec2 size(ENDER_W, ENDER_H);

	// Gravedad (solo para asentarse al aparecer)
	posEnder.y += ENDER_FALL_STEP;
	bool onGround = map->collisionMoveDown(posEnder, size, &posEnder.y);

	// Movimiento horizontal (igual que el zombie)
	moveTick++;
	if (onGround && moveTick >= ENDER_MOVE_EVERY)
	{
		moveTick = 0;
		posEnder.x += dir * ENDER_SPEED;

		bool wall = (dir < 0) ? map->collisionMoveLeft(posEnder, size)
			: map->collisionMoveRight(posEnder, size);

		int ts = map->getTileSize();
		int footX = (dir < 0) ? posEnder.x : posEnder.x + ENDER_W - 1;
		bool ground = map->isSolidTile(footX / ts, (posEnder.y + ENDER_H) / ts);

		if (wall || !ground)
		{
			posEnder.x -= dir * ENDER_SPEED;
			dir = -dir;
			sprite->changeAnimation(dir < 0 ? WALK_LEFT : WALK_RIGHT);
		}
	}

	// Teletransporte: cada 5 s, 2 tiles hacia donde camina, solo si puede
	teleportTimer += deltaTime;
	if (onGround && teleportTimer >= ENDER_TELEPORT_TIME)
	{
		teleportTimer = 0;
		int nx = posEnder.x + dir * ENDER_TELEPORT_DIST;
		if (canStandAt(nx, posEnder.y))
			posEnder.x = nx;
	}

	// Muerte por explosión
	if (bombManager->isInExplosion(getHitPos(), getHitSize()))
	{
		Audio::instance().play(Audio::ENDERMAN_DEATH);
		dying = true;
		dyingTimer = 0;
		sprite->changeAnimation(dir < 0 ? DIE_LEFT : DIE_RIGHT);
	}

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnder.x), float(tileMapDispl.y + posEnder.y)));
}

void Enderman::render()
{
	if (!dead)
		sprite->render();
}

void Enderman::setPosition(const glm::vec2& pos)
{
	posEnder = pos;
	spawnPos = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnder.x), float(tileMapDispl.y + posEnder.y)));
}

glm::ivec2 Enderman::getHitPos() const
{
	return glm::ivec2(posEnder.x + HIT_MARGIN_X, posEnder.y + HIT_MARGIN_Y);
}

glm::ivec2 Enderman::getHitSize() const
{
	return glm::ivec2(ENDER_W - 2 * HIT_MARGIN_X, ENDER_H - HIT_MARGIN_Y);
}

void Enderman::respawn()
{
	dead = false;
	dying = false;
	dyingTimer = 0;
	moveTick = 0;
	teleportTimer = 0;
	dir = -1;
	sprite->changeAnimation(WALK_LEFT);
	setPosition(spawnPos);
}

void Enderman::kill()
{
	if (!isAlive()) return;
	dying = true;
	dyingTimer = 0;
	sprite->changeAnimation(dir < 0 ? DIE_LEFT : DIE_RIGHT);
}