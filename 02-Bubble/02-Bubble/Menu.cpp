#include <GL/glew.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Menu.h"


namespace
{
	const glm::ivec2 BACKGROUND_SIZE(640, 512);
	const glm::ivec2 CURSOR_SIZE(24, 24);

	const glm::vec2 CURSOR_POSITIONS[4] = {
		glm::vec2(342.f, 253.f),
		glm::vec2(342.f, 287.f),
		glm::vec2(342.f, 325.f),
		glm::vec2(342.f, 365.f)
	};

	bool loadTexture(Texture& texture, const char* path)
	{
		if (!texture.loadFromFile(path, TEXTURE_PIXEL_FORMAT_RGBA))
		{
			std::cout << "Could not load " << path << std::endl;
			return false;
		}
		texture.setMinFilter(GL_NEAREST);
		texture.setMagFilter(GL_NEAREST);
		return true;
	}

	Sprite* createStaticSprite(const glm::ivec2& size, Texture* texture, ShaderProgram* program)
	{
		Sprite* sprite = Sprite::createSprite(size, glm::vec2(1.f, 1.f), texture, program);

		sprite->setNumberAnimations(1);
		sprite->setAnimationSpeed(0, 8);
		sprite->addKeyframe(0, glm::vec2(0.f, 0.f));
		sprite->changeAnimation(0);
		return sprite;
	}
}


Menu::Menu()
	: background(nullptr), cursor(nullptr), program(nullptr), option(0)
{
}

Menu::~Menu()
{
	delete background;
	delete cursor;
}

void Menu::init(ShaderProgram& shaderProgram)
{
	program = &shaderProgram;
	option = 0;

	loadTexture(backgroundTexture, "images/main_menu.png");
	loadTexture(cursorTexture, "images/cursor.png");

	background = createStaticSprite(BACKGROUND_SIZE, &backgroundTexture, program);
	background->setPosition(glm::vec2(0.f, 0.f));

	cursor = createStaticSprite(CURSOR_SIZE, &cursorTexture, program);
	cursor->setPosition(CURSOR_POSITIONS[option]);
}

Menu::Choice Menu::update(bool up, bool down, bool enter)
{
	if (up)
		option = (option + NUM_OPTIONS - 1) % NUM_OPTIONS;
	if (down)
		option = (option + 1) % NUM_OPTIONS;
	cursor->setPosition(CURSOR_POSITIONS[option]);

	if (enter)
		return static_cast<Choice>(option);
	return NONE;
}

void Menu::render()
{
	program->use();
	program->setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	program->setUniformMatrix4f("modelview", glm::mat4(1.f));
	program->setUniform2f("texCoordDispl", 0.f, 0.f);

	glActiveTexture(GL_TEXTURE0);
	backgroundTexture.use();
	background->render();

	cursorTexture.use();
	cursor->render();
}

void Menu::reset()
{
	option = 0;
}