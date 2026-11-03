#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Game.h"
#include "Instructions.h"


using namespace std;


void Game::init()
{
	bPlay = true;
	memset(keys, 0, sizeof(keys));
	memset(prevKeys, 0, sizeof(prevKeys));

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initShaders();
	menu.init(texProgram);
	instructions.init(texProgram);
	credits.init("images/credits.png", texProgram);
	currentState = MAIN_MENU;
	scene.init();
}

bool Game::update(int deltaTime)
{
	const bool up = justPressed(GLFW_KEY_UP);
	const bool down = justPressed(GLFW_KEY_DOWN);
	const bool enter = justPressed(GLFW_KEY_ENTER);
	const bool left = justPressed(GLFW_KEY_LEFT);
	const bool right = justPressed(GLFW_KEY_RIGHT);

	switch (currentState)
	{
	case MAIN_MENU:
		switch (menu.update(up, down, enter))
		{
		case Menu::PLAY:
			scene.restart();
			currentState = PLAYING;
			break;
		case Menu::INSTRUCTIONS:
			instructions.reset();
			currentState = INSTRUCTIONS;
			break;
		case Menu::CREDITS:
			currentState = CREDITS;
			break;
		case Menu::EXIT:
			bPlay = false;
			break;
		default:
			break;
		}
		break;

	case PLAYING:
		scene.update(deltaTime);
		break;

	case INSTRUCTIONS:
		instructions.update(left, right);
		if (enter)
			currentState = MAIN_MENU;
		break;

	case CREDITS:
		if (enter)
			currentState = MAIN_MENU;
		break;
	}

	memcpy(prevKeys, keys, sizeof(keys));
	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (currentState)
	{
	case MAIN_MENU:
		menu.render();
		break;

	case PLAYING:
		scene.render();
		break;

	case INSTRUCTIONS:
		instructions.render();
		break;

	case CREDITS:
		credits.render();
		break;
	}
}

void Game::keyPressed(int key)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		if (currentState == MAIN_MENU)
			bPlay = false;
		else
		{
			currentState = MAIN_MENU;
			menu.reset();
		}
	}
	keys[key] = true;
}

void Game::keyReleased(int key)
{
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
}

void Game::mousePress(int button)
{
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	return keys[key];
}

bool Game::justPressed(int key) const
{
	return keys[key] && !prevKeys[key];
}

void Game::initShaders()
{
	Shader vShader, fShader;

	if (!vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert"))
		cout << "Could not read shaders/texture.vert" << endl;
	if (!vShader.isCompiled())
		cout << "Vertex Shader Error" << endl << vShader.log() << endl << endl;

	if (!fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag"))
		cout << "Could not read shaders/texture.frag" << endl;
	if (!fShader.isCompiled())
		cout << "Fragment Shader Error" << endl << fShader.log() << endl << endl;

	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if (!texProgram.isLinked())
		cout << "Shader Linking Error" << endl << texProgram.log() << endl << endl;
	texProgram.bindFragmentOutput("outColor");

	vShader.free();
	fShader.free();

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
}