#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 1
#define INIT_PLAYER_Y_TILES 20


Scene::Scene()
{
	map = NULL;
	player = NULL;
	introSprite = NULL;
	gameOverSprite = NULL;
	lives = 3;
}

Scene::~Scene()
{
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
	if (introSprite != NULL) delete introSprite;
	if (gameOverSprite != NULL) delete gameOverSprite;
}

void Scene::init()
{
	initShaders();
	map = TileMap::createTileMap("levels/nivel01_Solidos.csv", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	MiddleMap = TileMap::createTileMap("levels/nivel01_Medio.csv", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	BackgroundMap = TileMap::createTileMap("levels/nivel01_Fondo.csv", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);

	spawnPos = glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize());
	player->setPosition(spawnPos);

	player->setTileMap(map);
	bombManager.init(glm::ivec2(SCREEN_X, SCREEN_Y), map, texProgram);
	player->setBombManager(&bombManager);
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;

	// Pantalla de intro del nivel
	introTex.loadFromFile("images/level1.png", TEXTURE_PIXEL_FORMAT_RGBA);
	introSprite = Sprite::createSprite(glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &introTex, &texProgram);
	introSprite->setNumberAnimations(1);
	introSprite->setAnimationSpeed(0, 8);
	introSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	introSprite->changeAnimation(0);
	introSprite->setPosition(glm::vec2(0.f, 0.f));

	// Pantalla de game over
	gameOverTex.loadFromFile("images/gameover.png", TEXTURE_PIXEL_FORMAT_RGBA);
	gameOverSprite = Sprite::createSprite(glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &gameOverTex, &texProgram);
	gameOverSprite->setNumberAnimations(1);
	gameOverSprite->setAnimationSpeed(0, 8);
	gameOverSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	gameOverSprite->changeAnimation(0);
	gameOverSprite->setPosition(glm::vec2(0.f, 0.f));

	state = STATE_INTRO;
	stateTimer = 0;
}

void Scene::update(int deltaTime)
{
	stateTimer += deltaTime;

	if (state == STATE_INTRO)
	{
		if (stateTimer >= 2000)
			state = STATE_PLAYING;
		return;
	}
	if (state == STATE_GAMEOVER)
	{
		if (stateTimer >= 1000 && Game::instance().getKey(GLFW_KEY_ENTER))
		{
			lives = 3;
			bombManager.clear();
			player->respawn(spawnPos);
			state = STATE_INTRO;
			stateTimer = 0;
		}
		return;
	}

	// STATE_PLAYING
	currentTime += deltaTime;
	player->update(deltaTime);
	bombManager.update(deltaTime);

	if (player->isDeathFinished())
	{
		lives--;
		bombManager.clear();
		stateTimer = 0;
		if (lives > 0)
		{
			player->respawn(spawnPos);
			state = STATE_INTRO;
		}
		else
			state = STATE_GAMEOVER;
	}
}

void Scene::render()
{
	glm::mat4 modelview;
	texProgram.use();

	if (state == STATE_INTRO) { introSprite->render();    return; }
	if (state == STATE_GAMEOVER) { gameOverSprite->render(); return; }

	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	BackgroundMap->render();
	MiddleMap->render();
	map->render();
	bombManager.render();
	player->render();
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}



