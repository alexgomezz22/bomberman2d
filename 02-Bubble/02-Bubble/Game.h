#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <GLFW/glfw3.h>
#include "Scene.h"
#include "Menu.h"
#include "Instructions.h"
#include "Screen.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 512


enum GameState { MAIN_MENU, PLAYING, INSTRUCTIONS, CREDITS };

class Game
{

private:
	Game() {}

public:
	static Game& instance()
	{
		static Game G;

		return G;
	}

	void init();
	bool update(int deltaTime);
	void render();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);
	bool justPressed(int key) const;
	bool getKey(int key) const;

private:
	void initShaders();
	void goToMenu();

private:
	bool bPlay;
	bool keys[GLFW_KEY_LAST + 1];
	bool prevKeys[GLFW_KEY_LAST + 1];
	GameState currentState;
	Scene scene;
	Menu menu;
	Screen credits;
	Instructions instructions;
	ShaderProgram texProgram;
};


#endif // _GAME_INCLUDE