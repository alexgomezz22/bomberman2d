#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP_RIGHT, FALL_RIGHT, JUMP_LEFT, FALL_LEFT, PUT_BOMB_LEFT, PUT_BOMB_RIGHT
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
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
	spritesheet.loadFromFile("images/SPRITESteveFINAL.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.125f, 0.125f), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(8);
	//col * 0.125, fila * 0.125
	
	// Quieto izquierda: fila 0, col 1
	sprite->setAnimationSpeed(STAND_LEFT, 8);
	sprite->addKeyframe(STAND_LEFT, glm::vec2(0.125f, 0.000f));

	// Quieto derecha: fila 0, col 0
	sprite->setAnimationSpeed(STAND_RIGHT, 8);
	sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.000f, 0.000f));

	// Andar izquierda: fila 5, cols 0-3
	sprite->setAnimationSpeed(MOVE_LEFT, 8);
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.000f, 0.625f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.125f, 0.625f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.250f, 0.625f));
	sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.375f, 0.625f));

	// Andar derecha: fila 1, cols 0-3
	sprite->setAnimationSpeed(MOVE_RIGHT, 8);
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.000f, 0.125f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.125f, 0.125f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.250f, 0.125f));
	sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.375f, 0.125f));


	sprite->setAnimationSpeed(JUMP_RIGHT, 8);
	sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.000f, 0.250f));


	sprite->setAnimationSpeed(JUMP_LEFT, 8);
	sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.375f, 0.250f));


	sprite->setAnimationSpeed(FALL_LEFT, 8);
	sprite->addKeyframe(FALL_LEFT, glm::vec2(0.250f, 0.250f));

	sprite->setAnimationSpeed(FALL_RIGHT, 8);
	sprite->addKeyframe(FALL_RIGHT, glm::vec2(0.125f, 0.250f));

	//poner bomba
	sprite->setAnimationSpeed(PUT_BOMB_RIGHT, 8);
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.000f, 0.375f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.125f, 0.375f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.250f, 0.375f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.375f, 0.375f));
	
	sprite->setAnimationSpeed(PUT_BOMB_LEFT, 8);
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.000f, 0.750f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.125f, 0.750f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.250f, 0.750f));
	sprite->addKeyframe(PUT_BOMB_LEFT, glm::vec2(0.375f, 0.750f));
		
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
	sprite->update(deltaTime);
	if(Game::instance().getKey(GLFW_KEY_LEFT))
	{
		facingLeft = true;
		if(sprite->animation() != MOVE_LEFT)
			sprite->changeAnimation(MOVE_LEFT);
		posPlayer.x -= 2;
		if(map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x += 2;
			sprite->changeAnimation(STAND_LEFT);
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		facingLeft = false;
		if(sprite->animation() != MOVE_RIGHT)
			sprite->changeAnimation(MOVE_RIGHT);
		posPlayer.x += 2;
		if(map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x -= 2;
			sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
		if(sprite->animation() == MOVE_LEFT)
			sprite->changeAnimation(STAND_LEFT);
		else if(sprite->animation() == MOVE_RIGHT)
			sprite->changeAnimation(STAND_RIGHT);
	}
	
	if(bJumping)
	{
		int jumpAnim = facingLeft ? JUMP_LEFT : JUMP_RIGHT;
		if (sprite->animation() != jumpAnim)
			sprite->changeAnimation(jumpAnim);

		jumpAngle += JUMP_ANGLE_STEP;
		if(jumpAngle == 180)
		{
			bJumping = false;
			posPlayer.y = startY;
		}
		else
		{
			posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
			if(jumpAngle > 90)
				bJumping = !map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y);
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		if (map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
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




