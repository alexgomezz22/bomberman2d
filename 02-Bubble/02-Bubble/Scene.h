#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#define NUM_LEVELS 5

#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "BombManager.h"
#include "Zombie.h"
#include "Enderman.h"
#include "Hud.h"
#include "PowerUp.h"
#include <vector>
#include "Creeper.h"


struct LevelData
{
	const char* solid, * middle, * background;
	int playerCol, playerRow;
	int nZombies;
	int zombieTiles[8][2];   // {columna, fila}
	int doorCol, doorRow;     // tile de arriba de la puerta; -1 si el nivel no tiene puerta
	int nEndermen;
	int endermanTiles[8][2];
	int nCreepers;
	int creeperTiles[4][2];
};



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
	void setPowerUp(int c, int r);
	void addLife();
	void restart();
	bool canLeaveGameOver() const;

private:
	void initShaders();
	PowerUpType getRandomType() const;
	void applyPowerUp(PowerUpType type);

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
	Texture gameOverTex;
	Sprite* gameOverSprite;

	void loadLevel(int n);
	int currentLevel;
	std::vector<Zombie*> zombies;
	std::vector<Enderman*> endermen;
	Texture introTex[NUM_LEVELS];      
	Sprite* introSprite[NUM_LEVELS];   
	bool doorOpen;

	Hud hud;
	int timeLeft;// ms que quedan
	int freezeTime = 0;
	std::vector<PowerUp*> powerups;
	std::vector<Creeper*> creepers;

};


#endif // _SCENE_INCLUDE

