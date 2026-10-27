#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include <cstdio>


#define SCREEN_X 32
#define SCREEN_Y 16

//#define INIT_PLAYER_X_TILES 1
//#define INIT_PLAYER_Y_TILES 20

#define DOOR_OPEN_TOP 344
#define DOOR_OPEN_BOTTOM 345

#define MAX_LIVES 5

// segundos para pasarse cada nivel
static const int LEVEL_TIME[NUM_LEVELS] = { 180, 150, 150, 180, 240 };   // 3:00, 2:30, 2:30, 3:00, 4:00

static const LevelData LEVELS[NUM_LEVELS] = {//los dos primeros nums son columna y fila del steve. los segundos numeros son el num de zombies y su pos. los siguientes dos la pos de la puerta, los siguientes endermans
	{ "levels/nivel01_Solidos.csv", "levels/nivel01_Medio.csv", "levels/nivel01_Fondo.csv", 1, 20, 1, { {19, 5} }, 26, 24, 1, { {5, 1} }  },
	{ "levels/nivel02_Solidos.csv", "levels/nivel02_Medio.csv", "levels/nivel02_Fondo.csv", 1, 20, 2, { {14, 7}, {28, 4} }, 30, 4 },
	{ "levels/nivel03_Solidos.csv", "levels/nivel03_Medio.csv", "levels/nivel03_Fondo.csv", 1, 20, 1, { {10, 5} }, 24, 2},
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
	doorOpen = false;
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
	bombManager.init(glm::ivec2(SCREEN_X, SCREEN_Y), NULL, texProgram, 0, this);

	// Pantallas de intro de cada nivel (imágenes de 64x64, dibujadas a 448x448 y centradas)
	for (int i = 0; i < NUM_LEVELS; i++)
	{
		char path[64];
		sprintf(path, "images/level%d.png", i + 1);
		introTex[i].loadFromFile(path, TEXTURE_PIXEL_FORMAT_RGBA);
		introTex[i].setMagFilter(GL_NEAREST);
		introTex[i].setMinFilter(GL_NEAREST);
		introSprite[i] = Sprite::createSprite(glm::ivec2(448, 448), glm::vec2(1.f, 1.f), &introTex[i], &texProgram);
		introSprite[i]->setNumberAnimations(1);
		introSprite[i]->setAnimationSpeed(0, 8);
		introSprite[i]->addKeyframe(0, glm::vec2(0.f, 0.f));
		introSprite[i]->changeAnimation(0);
		introSprite[i]->setPosition(glm::vec2(96.f, 32.f));
	}

	// Pantalla de game over
	gameOverTex.loadFromFile("images/gameover.png", TEXTURE_PIXEL_FORMAT_RGBA);
	gameOverSprite = Sprite::createSprite(glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &gameOverTex, &texProgram);
	gameOverSprite->setNumberAnimations(1);
	gameOverSprite->setAnimationSpeed(0, 8);
	gameOverSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	gameOverSprite->changeAnimation(0);
	gameOverSprite->setPosition(glm::vec2(0.f, 0.f));

	hud.init(texProgram);
	loadLevel(0);   // carga mapas, Steve y zombies del nivel 1, y pone el estado en INTRO
}

void Scene::update(int deltaTime)
{
	stateTimer += deltaTime;

	// Teclas 1-5: ir directamente a ese nivel
	static bool keyWasDown[NUM_LEVELS] = { false };
	for (int i = 0; i < NUM_LEVELS; i++)
	{
		bool down = Game::instance().getKey(GLFW_KEY_1 + i);
		bool pressed = down && !keyWasDown[i];// solo en el frame en que se pulsa, sino se carga 60 veces 
		keyWasDown[i] = down;
		if (pressed)
		{
			lives = 3;
			loadLevel(i);
			return;
		}
	}

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

	// Cuenta atrás: al llegar a 0, Steve muere
	if (!player->isDead())
	{
		if (freezeTime > 0) {
			freezeTime -= deltaTime;
			if (freezeTime < 0) freezeTime = 0;
		} else {
			timeLeft -= deltaTime;
			if (timeLeft <= 0)
			{
				timeLeft = 0;
				player->kill();
			}
		}
	}

	// Tecla K mata a todos los enemigos
	static bool kWasDown = false;
	bool kDown = Game::instance().getKey(GLFW_KEY_K);
	if (kDown && !kWasDown)
	{
		for (size_t i = 0; i < zombies.size(); i++) zombies[i]->kill();
		for (size_t i = 0; i < endermen.size(); i++) endermen[i]->kill();
	}
	kWasDown = kDown;

	bool allDead = true;
	glm::ivec2 p = player->getHitPos(), ps = player->getHitSize();

	for (size_t i = 0; i < zombies.size(); i++)
	{
		zombies[i]->update(deltaTime);
		if (!zombies[i]->isDead()) allDead = false;

		if (zombies[i]->isAlive() && !player->isDead())
		{
			glm::ivec2 z = zombies[i]->getHitPos(), zs = zombies[i]->getHitSize();
			if (p.x < z.x + zs.x && p.x + ps.x > z.x && p.y < z.y + zs.y && p.y + ps.y > z.y)
				player->kill();
		}
	}
	for (size_t i = 0; i < endermen.size(); i++)
	{
		endermen[i]->update(deltaTime);
		if (!endermen[i]->isDead()) allDead = false;

		if (endermen[i]->isAlive() && !player->isDead())
		{
			glm::ivec2 e = endermen[i]->getHitPos(), es = endermen[i]->getHitSize();
			if (p.x < e.x + es.x && p.x + ps.x > e.x && p.y < e.y + es.y && p.y + ps.y > e.y)
				player->kill();
		}
	}

	const LevelData& L = LEVELS[currentLevel];
	bool hasEnemies = !zombies.empty() || !endermen.empty();

	// Todos los enemigos muertos: se abre la puerta (o se pasa directo si el nivel no tiene)
	if (hasEnemies && allDead && !player->isDead() && !doorOpen)
	{
		if (L.doorCol < 0)
		{
			loadLevel(currentLevel + 1 < NUM_LEVELS ? currentLevel + 1 : 0);
			return;
		}
		doorOpen = true;
		map->setTile(L.doorCol, L.doorRow, 0);      // ya no es sólida
		map->setTile(L.doorCol, L.doorRow + 1, 0);
		MiddleMap->setTile(L.doorCol, L.doorRow, DOOR_OPEN_TOP + 1);      // se dibuja abierta
		MiddleMap->setTile(L.doorCol, L.doorRow + 1, DOOR_OPEN_BOTTOM + 1);
	}

	// Steve toca la puerta abierta: siguiente nivel
	if (doorOpen && !player->isDead())
	{
		int ts = map->getTileSize();
		int dx = L.doorCol * ts, dy = L.doorRow * ts;
		if (p.x < dx + ts && p.x + ps.x > dx && p.y < dy + 2 * ts && p.y + ps.y > dy)
		{
			loadLevel(currentLevel + 1 < NUM_LEVELS ? currentLevel + 1 : 0);   // falta pantalla de victoria tras el 5
			return;
		}
	}

	if (player->isDeathFinished())
	{
		lives--;
		if (lives > 0)
			loadLevel(currentLevel);   // recarga el nivel: bloques y enemigos como al inicio
		else
		{
			bombManager.clear();
			state = STATE_GAMEOVER;
			stateTimer = 0;
		}
	}

	for (auto it = powerups.begin(); it != powerups.end(); ) {
		(*it)->update(deltaTime);

		// Si el jugador colisiona con el powerup
		if ((*it)->checkCollision(player->getPosition(), player->getHitSize())) {
			applyPowerUp((*it)->getType());			// Aplicar power Up
			delete* it;                             // Liberar memoria del objeto
			it = powerups.erase(it);                // Eliminar del vector de forma segura
		}
		else {
			++it;
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
	for (PowerUp* p : powerups) {
		p->render();
	}
	bombManager.render();
	for (size_t i = 0; i < zombies.size(); i++) zombies[i]->render();
	for (size_t i = 0; i < endermen.size(); i++) endermen[i]->render();
	player->render();

	int enemies = 0;
	for (size_t i = 0; i < zombies.size(); i++)  if (zombies[i]->isAlive())  enemies++;
	for (size_t i = 0; i < endermen.size(); i++) if (endermen[i]->isAlive()) enemies++;
	hud.render(lives, enemies, player->getMaxBombs(), (timeLeft + 999) / 1000);
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
	doorOpen = false;
	timeLeft = LEVEL_TIME[n] * 1000;
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

	for (size_t i = 0; i < endermen.size(); i++) delete endermen[i];
	endermen.clear();
	for (int i = 0; i < L.nEndermen; i++)
	{
		Enderman* e = new Enderman();
		e->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
		e->setTileMap(map);
		e->setBombManager(&bombManager);
		e->setPosition(glm::vec2(L.endermanTiles[i][0] * map->getTileSize(),
			L.endermanTiles[i][1] * map->getTileSize()));
		endermen.push_back(e);
	}

	state = STATE_INTRO;
	stateTimer = 0;
}
PowerUpType Scene::getRandomType() const
{
	// Genera un entero entre 0 y 3
	int randomIdx = rand() % 4;	// Tenemos 4 tipos de PowerUps
	return static_cast<PowerUpType>(randomIdx);
}

void Scene::addLife()
{
	if (lives < MAX_LIVES) lives++;
}

void Scene::setPowerUp(int c, int r)
{
	//if ((rand() % 100) > 70) {
	if (true) {
		PowerUp* p = new PowerUp();
		p->init(glm::vec2(c, r), 16, getRandomType(), texProgram);
		powerups.push_back(p);
	}
}

void Scene::applyPowerUp(PowerUpType type)
{
	switch (type)
	{
	case BOMB_UP:
		// Aumenta el numero de bombas que se pueden poner a la vez
		player->increaseMaxBombs();
		break;

	case FIRE_UP:
		// Aumenta el radio de explosión de la bomba
		bombManager.increaseBombRange();
		break;

	case TIMER:
		// Congela el tiempo durante un rato
		freezeTime = 10000.0f;
		break;

	case LIFE_UP:
		// Añadir una vida
		addLife();
		break;
	}
}