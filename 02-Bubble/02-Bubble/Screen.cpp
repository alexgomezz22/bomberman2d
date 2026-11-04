#include <GL/glew.h>
#include <iostream>
#include "Screen.h"


namespace
{
	const glm::ivec2 SCREEN_SIZE(640, 512);
}


Screen::Screen()
	: sprite(nullptr), program(nullptr)
{
}

Screen::~Screen()
{
	delete sprite;
}

void Screen::init(const std::string& imagePath, ShaderProgram& shaderProgram)
{
	program = &shaderProgram;

	if (!texture.loadFromFile(imagePath, TEXTURE_PIXEL_FORMAT_RGBA))
		std::cout << "Could not load " << imagePath << std::endl;
	texture.setMinFilter(GL_NEAREST);
	texture.setMagFilter(GL_NEAREST);

	sprite = Sprite::createSprite(SCREEN_SIZE, glm::vec2(1.f, 1.f), &texture, program);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, 8);
	sprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	sprite->changeAnimation(0);
	sprite->setPosition(glm::vec2(0.f, 0.f));
}

void Screen::render()
{
	program->use();
	program->setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	program->setUniformMatrix4f("modelview", glm::mat4(1.f));
	program->setUniform2f("texCoordDispl", 0.f, 0.f);

	glActiveTexture(GL_TEXTURE0);
	texture.use();
	sprite->render();
}