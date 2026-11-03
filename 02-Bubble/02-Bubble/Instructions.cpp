#include <GL/glew.h>
#include <iostream>
#include "Instructions.h"


namespace
{
	const glm::ivec2 PAGE_SIZE(640, 512);

	const char* PAGE_PATHS[3] = {
		"images/instructions_how_to_play.png",
		"images/instructions_controls.png",
		"images/instructions_powerups.png"
	};
}


Instructions::Instructions()
	: program(nullptr), page(0)
{
	for (int i = 0; i < NUM_PAGES; i++)
		pageSprite[i] = nullptr;
}

Instructions::~Instructions()
{
	for (int i = 0; i < NUM_PAGES; i++)
		delete pageSprite[i];
}

void Instructions::init(ShaderProgram& shaderProgram)
{
	program = &shaderProgram;
	page = 0;

	for (int i = 0; i < NUM_PAGES; i++)
	{
		if (!pageTexture[i].loadFromFile(PAGE_PATHS[i], TEXTURE_PIXEL_FORMAT_RGBA))
			std::cout << "Could not load " << PAGE_PATHS[i] << std::endl;
		pageTexture[i].setMinFilter(GL_NEAREST);
		pageTexture[i].setMagFilter(GL_NEAREST);

		pageSprite[i] = Sprite::createSprite(PAGE_SIZE, glm::vec2(1.f, 1.f), &pageTexture[i], program);
		pageSprite[i]->setNumberAnimations(1);
		pageSprite[i]->setAnimationSpeed(0, 8);
		pageSprite[i]->addKeyframe(0, glm::vec2(0.f, 0.f));
		pageSprite[i]->changeAnimation(0);
		pageSprite[i]->setPosition(glm::vec2(0.f, 0.f));
	}
}

void Instructions::update(bool left, bool right)
{
	if (left && page > 0)
		page--;
	if (right && page < NUM_PAGES - 1)
		page++;
}

void Instructions::render()
{
	program->use();
	program->setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	program->setUniformMatrix4f("modelview", glm::mat4(1.f));
	program->setUniform2f("texCoordDispl", 0.f, 0.f);

	glActiveTexture(GL_TEXTURE0);
	pageTexture[page].use();
	pageSprite[page]->render();
}

void Instructions::reset()
{
	page = 0;
}