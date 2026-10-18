#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include <cstdio>


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 1
#define INIT_PLAYER_Y_TILES 20

static const LevelData LEVELS[NUM_LEVELS] = {//los dos primeros nums son columna y fila del steve. los segundos numeros son el num de zombies y su pos
	{ "levels/nivel01_Solidos.csv", "levels/nivel01_Medio.csv", "levels/nivel01_Fondo.csv", 1, 20, 1, { {19, 5} } },
	{ "levels/nivel02_Solidos.csv", "levels/nivel02_Medio.csv", "levels/nivel02_Fondo.csv", 1, 20, 2, { {14, 7}, {28, 4} } },
	{ "levels/nivel03_Solidos.csv", "levels/nivel03_Medio.csv", "levels/nivel03_Fondo.csv", 1, 20, 1, { {19, 5} } },
	{ "levels/nivel04_Solidos.csv", "levels/nivel04_Medio.csv", "levels/nivel04_Fondo.csv", 1, 20, 1, { {19, 5} } },
	{ "levels/nivel05_Solidos.csv", "levels/nivel05_Medio.csv", "levels/nivel05_Fondo.csv", 1, 20, 1, { {19, 5} } }
};

Scene::Scene()
{
	map = MiddleMap = BackgroundMap = NULL;
	player = NULL;
	gameOverSprite = NULL;
	for (int i = 0; i < NUM_LEVELS; i++) introSprite[i] = NULL;
	lives = 3;
	currentLevel = 0;
}

Scene::~Scene()
{
	texProgram.free();
	delete map; delete MiddleMap; delete BackgroundMap;
	delete player;
	for (int i = 0; i < NUM_LEVELS; i++) delete introSprite[i];
	delete gameOverSprite;
	for (size_t i = 0; i < zombies.size(); i++) delete zombies[i];
}

void Scene::init()
{
	initShaders();
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);
	currentTime = 0.0f;

	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setBombManager(&bombManager);
	bombManager.init(glm::ivec2(SCREEN_X, SCREEN_Y), NULL, texProgram, 0);

	// Pantallas de intro de cada nivel (imágenes de 64x64, dibujadas a 448x448 y centradas)
	for (int i = 0; i < NUM_LEVELS; i++)
	{
		char path[64];
		sprintf(path, "images/level%d.png", i + 1);
		introTex[i].loadFromFile(path, TEXTURE_PIXEL_FORMAT_RGBA);
		introTex[i].setMagFilter(GL_NEAREST);                 // <- nuevo
		introTex[i].setMinFilter(GL_NEAREST);                 // <- nuevo
		introSprite[i] = Sprite::createSprite(glm::ivec2(448, 448), glm::vec2(1.f, 1.f), &introTex[i], &texProgram);   // <- 448x448
		introSprite[i]->setNumberAnimations(1);
		introSprite[i]->setAnimationSpeed(0, 8);
		introSprite[i]->addKeyframe(0, glm::vec2(0.f, 0.f));
		introSprite[i]->changeAnimation(0);
		introSprite[i]->setPosition(glm::vec2(96.f, 16.f));   // <- centrada
	}

	// Pantalla de game over (sin cambios)
	gameOverTex.loadFromFile("images/gameover.png", TEXTURE_PIXEL_FORMAT_RGBA);
	gameOverSprite = Sprite::createSprite(glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &gameOverTex, &texProgram);
	gameOverSprite->setNumberAnimations(1);
	gameOverSprite->setAnimationSpeed(0, 8);
	gameOverSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	gameOverSprite->changeAnimation(0);
	gameOverSprite->setPosition(glm::vec2(0.f, 0.f));

	loadLevel(0);   // carga mapas, Steve y zombies del nivel 1, y pone el estado en INTRO
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
			loadLevel(0);
		}
		return;
	}

	// STATE_PLAYING
	currentTime += deltaTime;
	player->update(deltaTime);
	bombManager.update(deltaTime);

	bool allDead = true;
	for (size_t i = 0; i < zombies.size(); i++)
	{
		zombies[i]->update(deltaTime);
		if (!zombies[i]->isDead()) allDead = false;

		if (zombies[i]->isAlive() && !player->isDead())
		{
			glm::ivec2 p = player->getHitPos(), ps = player->getHitSize();
			glm::ivec2 z = zombies[i]->getHitPos(), zs = zombies[i]->getHitSize();
			if (p.x < z.x + zs.x && p.x + ps.x > z.x && p.y < z.y + zs.y && p.y + ps.y > z.y)
				player->kill();
		}
	}

	// Nivel superado (si Steve ha muerto a la vez, cuenta la muerte)
	if (!zombies.empty() && allDead && !player->isDead())
	{
		if (currentLevel + 1 < NUM_LEVELS)
			loadLevel(currentLevel + 1);
		else
			loadLevel(0);          // TODO: pantalla de victoria
		return;
	}

	if (player->isDeathFinished())
	{
		lives--;
		if (lives > 0)
			loadLevel(currentLevel);   // recarga el nivel: bloques y zombies como al inicio
		else
		{
			bombManager.clear();
			state = STATE_GAMEOVER;
			stateTimer = 0;
		}
	}
}

void Scene::render()
{
	glm::mat4 modelview;
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	if (state == STATE_INTRO) { introSprite[currentLevel]->render(); return; }
	if (state == STATE_GAMEOVER) { gameOverSprite->render();            return; }

	BackgroundMap->render();
	MiddleMap->render();
	map->render();
	bombManager.render();
	for (size_t i = 0; i < zombies.size(); i++) zombies[i]->render();
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

void Scene::loadLevel(int n)
{
	currentLevel = n;
	const LevelData& L = LEVELS[n];
	glm::ivec2 disp(SCREEN_X, SCREEN_Y);

	delete map; delete MiddleMap; delete BackgroundMap;
	map = TileMap::createTileMap(L.solid, glm::vec2(disp), texProgram);
	MiddleMap = TileMap::createTileMap(L.middle, glm::vec2(disp), texProgram);
	BackgroundMap = TileMap::createTileMap(L.background, glm::vec2(disp), texProgram);

	bombManager.clear();
	bombManager.setMap(map);
	player->setTileMap(map);
	spawnPos = glm::vec2(L.playerCol * map->getTileSize(), L.playerRow * map->getTileSize());
	player->respawn(spawnPos);

	for (size_t i = 0; i < zombies.size(); i++) delete zombies[i];
	zombies.clear();
	for (int i = 0; i < L.nZombies; i++)
	{
		Zombie* z = new Zombie();
		z->init(disp, texProgram);
		z->setPosition(glm::vec2(L.zombieTiles[i][0] * map->getTileSize(), L.zombieTiles[i][1] * map->getTileSize()));
		z->setTileMap(map);
		z->setBombManager(&bombManager);
		zombies.push_back(z);
	}

	state = STATE_INTRO;
	stateTimer = 0;
}


