#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "BombManager.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.

enum SceneState { STATE_INTRO, STATE_PLAYING, STATE_GAMEOVER };

class Scene
{

public:
	Scene();
	~Scene();

	void init();
	void update(int deltaTime);
	void render();

private:
	void initShaders();

private:
	TileMap *map;			// Mapa de colisiones (dense layer)
	TileMap *MiddleMap;		// Mapa del medio (cosas entre el personaje y el fondo)
	TileMap *BackgroundMap;	// Mapa del fondo (paisaje)
	Player *player;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	BombManager bombManager;
	int lives;
	glm::vec2 spawnPos;
	SceneState state;
	int stateTimer;
	Texture introTex, gameOverTex;
	Sprite* introSprite, * gameOverSprite;


};


#endif // _SCENE_INCLUDE

