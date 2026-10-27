#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4
#define BOMB_ANIM_SPEED 16
#define BOMB_ANIM_TIME (4 * 1000 / BOMB_ANIM_SPEED)   // 250 ms


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP_RIGHT, FALL_RIGHT, JUMP_LEFT, FALL_LEFT, PUT_BOMB_LEFT, PUT_BOMB_RIGHT, DIE_LEFT, DIE_RIGHT
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
	bombManager = NULL;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
}
// Recommend to put all the enemies in the same spritesheet
void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	bJumping = false;
	facingLeft = false;
	bPuttingBomb = false;
	maxBombs = 2;
	bBWasDown = false;
	dead = false;
	bombTimer = 0;
	deathTimer = 0;
	spritesheet.loadFromFile("images/SPRITESteveFINAL.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.125f, 0.125f), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(12);
	//col * 0.125, fila * 0.125
	
	// Quieto izquierda: fila 0, col 1
	sprite->setAnimationSpeed(STAND_LEFT, 8);
	sprite->addKeyframe(STAND_LEFT, glm::vec2(0.125f, 0.000f));

	// Quieto derecha: fila 0, col 0
	sprite->setAnimationSpeed(STAND_RIGHT, 8);
	sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.000f, 0.000f));

	// Andar izquierda: fila 5, cols 0-3
	sprite->setAnimationSpeed(MOVE_LEFT, 8);
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.375f, 0.625f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.250f, 0.625f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.125f, 0.625f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.000f, 0.625f));

	// Andar derecha: fila 1, cols 0-3
	sprite->setAnimationSpeed(MOVE_RIGHT, 8);
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.375f, 0.125f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.250f, 0.125f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.125f, 0.125f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.000f, 0.125f));


	sprite->setAnimationSpeed(JUMP_RIGHT, 8);
	sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.000f, 0.250f));


	sprite->setAnimationSpeed(JUMP_LEFT, 8);
	sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.375f, 0.250f));


	sprite->setAnimationSpeed(FALL_LEFT, 8);
	sprite->addKeyframe(FALL_LEFT, glm::vec2(0.250f, 0.250f));

	sprite->setAnimationSpeed(FALL_RIGHT, 8);
	sprite->addKeyframe(FALL_RIGHT, glm::vec2(0.125f, 0.250f));

	//poner bomba
	sprite->setAnimationSpeed(PUT_BOMB_RIGHT, BOMB_ANIM_SPEED);
	sprite->addKeyframe(PUT_BOMB_RIGHT, glm::vec2(0.000f, 0.375f));
	sprite->addKeyframe(PUT_BOMB_RIGHT, glm::vec2(0.125f, 0.375f));
	sprite->addKeyframe(PUT_BOMB_RIGHT, glm::vec2(0.250f, 0.375f));
	sprite->addKeyframe(PUT_BOMB_RIGHT, glm::vec2(0.375f, 0.375f));
	
	sprite->setAnimationSpeed(PUT_BOMB_LEFT, BOMB_ANIM_SPEED);
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.375f, 0.750f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.250f, 0.750f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.125f, 0.750f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.000f, 0.750f));

	//morir
	sprite->setAnimationSpeed(DIE_LEFT, DEATH_ANIM_SPEED);
	sprite->addKeyframe(DIE_LEFT, glm::vec2(0.000f, 0.875f));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(0.125f, 0.875f));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(0.250f, 0.875f));
	sprite->addKeyframe(DIE_LEFT, glm::vec2(0.375f, 0.875f));

	sprite->setAnimationSpeed(DIE_RIGHT, DEATH_ANIM_SPEED);
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(0.000f, 0.500f));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(0.125f, 0.500f));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(0.250f, 0.500f));
	sprite->addKeyframe(DIE_RIGHT, glm::vec2(0.375f, 0.500f));
		
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
	if (dead)
	{
		if (deathTimer < DEATH_LAST_FRAME_TIME)
			sprite->update(deltaTime);
		deathTimer += deltaTime;
		return;
	}

	sprite->update(deltaTime);

	std::vector<Bomb*>& bombs = bombManager->getBombs();
	for (size_t i = 0; i < bombs.size(); i++)
		if (!bombs[i]->isSolid() && !bombs[i]->overlaps(posPlayer, glm::ivec2(32, 32)))
			bombs[i]->setSolid(true);

	// Click de B
	// Click de B
	bool bDown = Game::instance().getKey(GLFW_KEY_B);
	if (bDown && !bBWasDown && !bPuttingBomb && bombManager->countActive() < maxBombs)
	{
		int col = (posPlayer.x + 16) / 16;
		int row = (posPlayer.y + 31) / 16;

		if (bombManager->addBomb(col, row))   // false si ya hay una bomba en ese tile
		{
			int anim = sprite->animation();
			bool inAir = bJumping || anim == JUMP_LEFT || anim == JUMP_RIGHT
				|| anim == FALL_LEFT || anim == FALL_RIGHT;
			if (!inAir)
			{
				bPuttingBomb = true;
				bombTimer = BOMB_ANIM_TIME;
				sprite->changeAnimation(facingLeft ? PUT_BOMB_LEFT : PUT_BOMB_RIGHT);
			}
		}
	}
	bBWasDown = bDown;

	// Fin de la animación de poner bomba
	if (bPuttingBomb)
	{
		bombTimer -= deltaTime;
		if (bombTimer <= 0)
		{
			bPuttingBomb = false;
			sprite->changeAnimation(facingLeft ? STAND_LEFT : STAND_RIGHT);
		}
	}

	// Movimiento horizontal
	if (bPuttingBomb)
	{
		// quieto mientras dura la animación
	}
	else if (Game::instance().getKey(GLFW_KEY_LEFT))
	{
		facingLeft = true;
		if (sprite->animation() != MOVE_LEFT)
			sprite->changeAnimation(MOVE_LEFT);
		posPlayer.x -= 2;
		if (map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)) || bombBlocks(posPlayer))
		{
			posPlayer.x += 2;
			sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		facingLeft = false;
		if (sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
		posPlayer.x += 2;
		if (map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)) || bombBlocks(posPlayer))
		{
			posPlayer.x -= 2;
			sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
		if (sprite->animation() == MOVE_LEFT)
			sprite->changeAnimation(STAND_LEFT);
		else if (sprite->animation() == MOVE_RIGHT)
			sprite->changeAnimation(STAND_RIGHT);
	}

	// Salto y caída
	if (bJumping)
	{
		int jumpAnim = facingLeft ? JUMP_LEFT : JUMP_RIGHT;	
		if (sprite->animation() != jumpAnim)		// If its not correctly draw, the animation changes to the correct one
			sprite->changeAnimation(jumpAnim);

		jumpAngle += JUMP_ANGLE_STEP;
		if (jumpAngle == 180)
		{
			bJumping = false;		// then it stops jumping
			posPlayer.y = startY;
		}
		else
		{
			int oldY = posPlayer.y;
			// Calculamos la posición teórica a la que debería ir en este frame
			int targetY = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));

			if (jumpAngle <= 90)
			{
				// Fase de subida: el desplazamiento es hacia arriba, no hay problema de gravedad
				posPlayer.y = targetY;
				if (bombBlocks(posPlayer) || map->collisionMoveUp(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
				{
					posPlayer.y = oldY;
					bJumping = false;
				}
			}
			else // jumpAngle > 90
			{
				posPlayer.y = oldY;
				bool collided = false;

				while (posPlayer.y < targetY)
				{
					int step = (targetY - posPlayer.y > FALL_STEP) ? FALL_STEP : (targetY - posPlayer.y);
					posPlayer.y += step;

					if (map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y) || bombLandDown())
					{
						collided = true;
						break;
					}
				}
				bJumping = !collided;
			}
		}
	}
	else
	{	// The player isn't in the middle of a jump
		posPlayer.y += FALL_STEP;
		bool onGround = map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y);
		if (!onGround)
			onGround = bombLandDown();

		if (onGround)
		{
			int anim = sprite->animation();
			if (anim == FALL_RIGHT || anim == JUMP_RIGHT || anim == FALL_LEFT || anim == JUMP_LEFT)
				sprite->changeAnimation(facingLeft ? STAND_LEFT : STAND_RIGHT);

			if (Game::instance().getKey(GLFW_KEY_UP))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
			}
		}
		else
		{
			int fallAnim = facingLeft ? FALL_LEFT : FALL_RIGHT;
			if (sprite->animation() != fallAnim)
				sprite->changeAnimation(fallAnim);
		}
	}

	glm::ivec2 hitPos(posPlayer.x + HIT_OFFSET_X, posPlayer.y + HIT_OFFSET_Y);
	if (bombManager->isInExplosion(hitPos, glm::ivec2(HIT_W, HIT_H)))
		die();

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::render()
{
	sprite->render();
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

bool Player::bombBlocks(const glm::ivec2& pos)
{
	std::vector<Bomb*>& bombs = bombManager->getBombs();
	for (size_t i = 0; i < bombs.size(); i++)
		if (bombs[i]->isSolid() && bombs[i]->overlaps(pos, glm::ivec2(32, 32)))
			return true;
	return false;
}

bool Player::bombLandDown()
{
	std::vector<Bomb*>& bombs = bombManager->getBombs();
	for (size_t i = 0; i < bombs.size(); i++)
	{
		if (!bombs[i]->isSolid() || !bombs[i]->overlaps(posPlayer, glm::ivec2(32, 32)))
			continue;
		int bombY = bombs[i]->getPosition().y;
		if (posPlayer.y + 32 - bombY <= 8)   // los pies acaban de pasar la parte de arriba
		{
			posPlayer.y = bombY - 32;
			return true;
		}
	}
	return false;
}

void Player::die()
{
	dead = true;
	deathTimer = 0;
	bPuttingBomb = false;
	sprite->changeAnimation(facingLeft ? DIE_LEFT : DIE_RIGHT);
}

void Player::respawn(const glm::vec2 &pos)
{
	dead = false;
	deathTimer = 0;
	bJumping = false;
	bPuttingBomb = false;
	bBWasDown = false;
	facingLeft = false;
	sprite->changeAnimation(STAND_RIGHT);
	setPosition(pos);
}