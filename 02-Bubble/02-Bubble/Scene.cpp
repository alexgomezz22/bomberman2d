#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include <cstdio>
#include "Audio.h"


#define SCREEN_X 32
#define SCREEN_Y 16

//#define INIT_PLAYER_X_TILES 1
//#define INIT_PLAYER_Y_TILES 20

#define MAX_LIVES 5

#define CAMERA_ZOOM 2     // 2  se ven 18x14 tiles;
#define VIEW_W 576        // tamaño del hueco del marco
#define VIEW_H 448

// segundos para pasarse cada nivel
static const int LEVEL_TIME[NUM_LEVELS] = { 180, 150, 150, 180, 240 };   // 3:00, 2:30, 2:30, 3:00, 4:00

static const LevelData LEVELS[NUM_LEVELS] = {//los dos primeros nums son columna y fila del steve. los segundos numeros son el num de zombies y su pos. los siguientes dos la pos de la puerta, los siguientes endermans, los siguientes creepers
	{ "levels/nivel01_Solidos.csv", "levels/nivel01_Medio.csv", "levels/nivel01_Fondo.csv", 1, 20, 1, { {30, 1}}, 26, 24, 1, { {5, 1} }, 1, { {19, 5} } },
	{ "levels/nivel02_Solidos.csv", "levels/nivel02_Medio.csv", "levels/nivel02_Fondo.csv", 1, 20, 2, { {30, 20}, {25, 8} }, 30, 4 , 1, { {14,5} }, 2, { {2,7}, {30,11} }},
	{ "levels/nivel03_Solidos.csv", "levels/nivel03_Medio.csv", "levels/nivel03_Fondo.csv", 3, 23, 2, { {11, 24}, {3, 6} }, 24, 2, 1, { {18, 22} }, 2, { {10, 5},{24, 8} }},
	{ "levels/nivel04_Solidos.csv", "levels/nivel04_Medio.csv", "levels/nivel04_Fondo.csv", 1, 7, 2, { {19, 5} , {32, 21}}, 4, 24, 1, {{6, 1}}, 1, { {32, 1} } },
	{ "levels/nivel05_Solidos.csv", "levels/nivel05_Medio.csv", "levels/nivel05_Fondo.csv", 5, 22, 0, { }, 3, 23, 4, { {16 , 21}, {24 ,21}, {5 , 8}, {23 , 7} }, 0, { } }
};

// tiles de la puerta abierta de cada nivel
static const int DOOR_OPEN_TILES[NUM_LEVELS][6] = {
	{ 344, 345 },
	{ 260, 260 },
	{ 260, 260 },
	{ 340, 340, 340, 340},
	{ 340, 340, 340, 340}
};
static const int DOOR_W[NUM_LEVELS] = { 1, 1, 1, 4, 4 };// ancho
static const int DOOR_H[NUM_LEVELS] = { 2, 2, 2, 1, 1};//alto
Scene::Scene()
{
	map = MiddleMap = BackgroundMap = NULL;
	player = NULL;
	gameOverSprite = NULL;
	victorySprite = NULL;
	shieldSprite = NULL;
	for (int i = 0; i < NUM_LEVELS; i++) introSprite[i] = NULL;
	lives = 3;
	currentLevel = 0;
	doorOpen = false;
	enderDragon = NULL;
}

Scene::~Scene()
{
	texProgram.free();
	delete map; delete MiddleMap; delete BackgroundMap;
	delete player;
	for (int i = 0; i < NUM_LEVELS; i++) delete introSprite[i];
	delete gameOverSprite;
	delete victorySprite;
	delete shieldSprite;
	for (size_t i = 0; i < zombies.size(); i++) delete zombies[i];
	for (size_t i = 0; i < creepers.size(); i++) delete creepers[i];
	if (enderDragon != NULL) delete enderDragon;
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
		introTex[i].setMagFilter(GL_LINEAR);
		introTex[i].setMinFilter(GL_LINEAR);
		introSprite[i] = Sprite::createSprite(glm::ivec2(640, 512), glm::vec2(1.f, 1.f), &introTex[i], &texProgram);
		introSprite[i]->setNumberAnimations(1);
		introSprite[i]->setAnimationSpeed(0, 8);
		introSprite[i]->addKeyframe(0, glm::vec2(0.f, 0.f));
		introSprite[i]->changeAnimation(0);
		introSprite[i]->setPosition(glm::vec2(0.f, 0.f));
	}

	// Pantalla de game over
	gameOverTex.loadFromFile("images/gameover.png", TEXTURE_PIXEL_FORMAT_RGBA);
	gameOverSprite = Sprite::createSprite(glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &gameOverTex, &texProgram);
	gameOverSprite->setNumberAnimations(1);
	gameOverSprite->setAnimationSpeed(0, 8);
	gameOverSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	gameOverSprite->changeAnimation(0);
	gameOverSprite->setPosition(glm::vec2(0.f, 0.f));

	// Pantalla de victoria
	victoryTex.loadFromFile("images/congratulations.png", TEXTURE_PIXEL_FORMAT_RGBA);
	victorySprite = Sprite::createSprite(glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f), &victoryTex, &texProgram);
	victorySprite->setNumberAnimations(1);
	victorySprite->setAnimationSpeed(0, 8);
	victorySprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	victorySprite->changeAnimation(0);
	victorySprite->setPosition(glm::vec2(0.f, 0.f));

	// Escudo del God Mode
	shieldTex.loadFromFile("images/shield.png", TEXTURE_PIXEL_FORMAT_RGBA);
	shieldTex.setMinFilter(GL_NEAREST);
	shieldTex.setMagFilter(GL_NEAREST);
	shieldSprite = Sprite::createSprite(glm::ivec2(8, 8), glm::vec2(1.f, 1.f), &shieldTex, &texProgram);
	shieldSprite->setNumberAnimations(1);
	shieldSprite->setAnimationSpeed(0, 8);
	shieldSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	shieldSprite->changeAnimation(0);

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
		bool pressed = down && !keyWasDown[i];// solo en el frame en que se pulsa, sino se carga muchas veces 
		keyWasDown[i] = down;
		if (pressed)
		{
			lives = 3;
			loadLevel(i);
			bombManager.resetBombRange();
			player->resetNumberBombs();
			return;
		}
	}

	if (state == STATE_INTRO)
	{
		if (stateTimer >= 2000)
			state = STATE_PLAYING;
		return;
	}
	if (state == STATE_GAMEOVER || state == STATE_VICTORY)
		return;

	// STATE_PLAYING
	currentTime += deltaTime;
	player->update(deltaTime, godMode);
	bombManager.update(deltaTime);

	// Cuenta atrás: al llegar a 0, Steve muere
	if (!player->isDead())
	{
		if (freezeTime > 0) {
			freezeTime -= deltaTime;
			if (freezeTime < 0) freezeTime = 0;
		} else {
			timeLeft -= deltaTime;
			if (timeLeft <= 0 && !godMode)
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
		for (size_t i = 0; i < creepers.size(); i++) creepers[i]->kill();
		if (enderDragon != NULL) enderDragon->kill();
	}
	kWasDown = kDown;

	// Tecla G activa god mode
	if (Game::instance().justPressed(GLFW_KEY_G))
		godMode = !godMode;

	bool allDead = true;
	glm::ivec2 p = player->getHitPos(), ps = player->getHitSize();

	for (size_t i = 0; i < zombies.size(); i++)
	{
		zombies[i]->update(deltaTime);
		if (!zombies[i]->isDead()) allDead = false;

		if (zombies[i]->isAlive() && !player->isDead())
		{
			glm::ivec2 z = zombies[i]->getHitPos(), zs = zombies[i]->getHitSize();
			if ((p.x < z.x + zs.x && p.x + ps.x > z.x && p.y < z.y + zs.y && p.y + ps.y > z.y) && !godMode)
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
			if ((p.x < e.x + es.x && p.x + ps.x > e.x && p.y < e.y + es.y && p.y + ps.y > e.y) && !godMode)
				player->kill();
		}
	}
	for (size_t i = 0; i < creepers.size(); i++)
	{
		creepers[i]->update(deltaTime);
		if (!creepers[i]->isDead()) allDead = false;

		if (creepers[i]->isAlive() && !player->isDead())
		{
			glm::ivec2 c = creepers[i]->getHitPos(), cs = creepers[i]->getHitSize();
			if ((p.x < c.x + cs.x && p.x + ps.x > c.x && p.y < c.y + cs.y && p.y + ps.y > c.y) && !godMode)
				player->kill();
		}
	}
	if (enderDragon != NULL && enderDragon->isAlive()) {
		enderDragon->update(deltaTime, player->getPosition());
		allDead = false;

		// Colisión Steve vs Dragón
		glm::ivec2 dPos = enderDragon->getHitPos(), dSize = enderDragon->getHitSize();
		if ((p.x < dPos.x + dSize.x && p.x + ps.x > dPos.x && p.y < dPos.y + dSize.y && p.y + ps.y > dPos.y) && !godMode){
			player->kill();
		}

		// Colisión Steve vs Bolas de fuego
		for (auto& fb : enderDragon->getFireballs()) {
			if (fb.active) {
				if ((p.x < fb.pos.x + 16 && p.x + ps.x > fb.pos.x && p.y < fb.pos.y + 16 && p.y + ps.y > fb.pos.y) && !godMode){
					player->kill();
				}
			}
		}
	}
	if (enderDragon != NULL) {
		enderDragon->update(deltaTime, glm::vec2(player->getHitPos()));

		if (!enderDragon->isAlive() && !enderDragon->isDying()) {
			delete enderDragon;
			enderDragon = NULL;
		}
	}

	const LevelData& L = LEVELS[currentLevel];
	bool hasEnemies = !zombies.empty() || !endermen.empty() || !creepers.empty();

	// Todos los enemigos muertos: se abre la puerta (o se pasa directo si el nivel no tiene)
	if (hasEnemies && allDead && !player->isDead() && !doorOpen)
	{
		if (L.doorCol < 0)
		{
			loadLevel(currentLevel + 1 < NUM_LEVELS ? currentLevel + 1 : 0);
			return;
		}
		doorOpen = true;
		int w = DOOR_W[currentLevel], h = DOOR_H[currentLevel];
		for (int r = 0; r < h; r++)
			for (int c = 0; c < w; c++)
			{
				map->setTile(L.doorCol + c, L.doorRow + r, 0);
				MiddleMap->setTile(L.doorCol + c, L.doorRow + r, DOOR_OPEN_TILES[currentLevel][r * w + c] + 1);
			}
	}

	// Steve toca la puerta abierta: siguiente nivel
	if (doorOpen && !player->isDead())
	{
		int ts = map->getTileSize();
		int dx = L.doorCol * ts, dy = L.doorRow * ts;
		int dw = DOOR_W[currentLevel] * ts, dh = DOOR_H[currentLevel] * ts;
		if (p.x < dx + dw && p.x + ps.x > dx && p.y < dy + dh && p.y + ps.y > dy)
		{
			if (currentLevel + 1 >= NUM_LEVELS)
			{
				bombManager.clear();
				bombManager.resetBombRange();
				player->resetNumberBombs();
				state = STATE_VICTORY;
				stateTimer = 0;
			}
			else
				loadLevel(currentLevel + 1);
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
			// Reiniciar los powerups
			bombManager.resetBombRange();	// Reiniciar rango bombas
			player->resetNumberBombs();		// Reiniciar numero bombas
											// Las vidas ya se reinician en otro lugar
		}
	}

	for (auto it = powerups.begin(); it != powerups.end(); ) {
		(*it)->update(deltaTime);

		// Si el jugador colisiona con el powerup
		if ((*it)->checkCollision(player->getPosition(), player->getHitSize())) {
			applyPowerUp((*it)->getType());			// Aplicar power Up
			Audio::instance().play(Audio::POWERUP);
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
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	texProgram.setUniformMatrix4f("modelview", glm::mat4(1.0f));
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	if (state == STATE_INTRO) { introSprite[currentLevel]->render(); return; }
	if (state == STATE_GAMEOVER) { gameOverSprite->render();            return; }
	if (state == STATE_VICTORY) { victorySprite->render(); return; }

	// Cámara ortogonal: sigue a Steve sin enseñar nada fuera del mapa
	glm::ivec2 hp = player->getHitPos(), hs = player->getHitSize();
	float viewW = float(VIEW_W) / CAMERA_ZOOM;
	float viewH = float(VIEW_H) / CAMERA_ZOOM;
	float mapW = map->getMapSize().x * map->getTileSize();
	float mapH = map->getMapSize().y * map->getTileSize();

	float camX = hp.x + hs.x / 2.f - viewW / 2.f;
	float camY = hp.y + hs.y / 2.f - viewH / 2.f;
	camX = std::max(0.f, std::min(camX, mapW - viewW));
	camY = std::max(0.f, std::min(camY, mapH - viewH));
	camX = floorf(camX);
	camY = floorf(camY);

	glm::mat4 cam = glm::translate(glm::mat4(1.f),
		glm::vec3(SCREEN_X * (1 - CAMERA_ZOOM) - CAMERA_ZOOM * camX,
			SCREEN_Y * (1 - CAMERA_ZOOM) - CAMERA_ZOOM * camY, 0.f));
	cam = glm::scale(cam, glm::vec3(CAMERA_ZOOM, CAMERA_ZOOM, 1.f));

	// El mundo se dibuja con la cámara dentro de la proyección (modelview queda en identidad)
	texProgram.setUniformMatrix4f("projection", projection * cam);

	BackgroundMap->render();
	MiddleMap->render();
	map->render();
	for (PowerUp* p : powerups) {
		p->render();
	}
	bombManager.render();
	for (size_t i = 0; i < zombies.size(); i++) zombies[i]->render();
	for (size_t i = 0; i < endermen.size(); i++) endermen[i]->render();
	for (size_t i = 0; i < creepers.size(); i++) creepers[i]->render();
	player->render();
	if (godMode) {
		shieldSprite->setPosition(glm::vec2(player->getPosition()) + glm::vec2(25.f, 15.f));
		shieldSprite->render();
	}

	int enemies = 0;
	for (size_t i = 0; i < zombies.size(); i++)  if (zombies[i]->isAlive())  enemies++;
	for (size_t i = 0; i < endermen.size(); i++) if (endermen[i]->isAlive()) enemies++;
	for (size_t i = 0; i < creepers.size(); i++) if (creepers[i]->isAlive()) enemies++;
	if (enderDragon != NULL && enderDragon->isAlive()) enemies++;

	if (enderDragon != NULL) enderDragon->render();


	// El marco y el HUD sin zoom
	texProgram.setUniformMatrix4f("projection", projection);

	if (enderDragon != NULL && enderDragon->isAlive())
	{
		hud.renderBossBar(enderDragon->getHP(), texProgram);
	}

	hud.render(lives, enemies, player->getMaxBombs(), bombManager.getBombRange(), (timeLeft + 999) / 1000);
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

	for (size_t i = 0; i < creepers.size(); i++) delete creepers[i];
	creepers.clear();
	for (int i = 0; i < L.nCreepers; i++)
	{
		Creeper* c = new Creeper();
		c->init(disp, texProgram);
		c->setTileMap(map);
		c->setBombManager(&bombManager);
		c->setPlayer(player);
		c->setPosition(glm::vec2(L.creeperTiles[i][0] * map->getTileSize(),
			L.creeperTiles[i][1] * map->getTileSize()));
		creepers.push_back(c);
	}

	if (enderDragon != NULL) {
		delete enderDragon;
		enderDragon = NULL;
	}

	// Spawnear jefe SOLO en el nivel 5
	if (n == 4) {
		enderDragon = new Dragon();
		enderDragon->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
		enderDragon->setTileMap(map);
		enderDragon->setBombManager(&bombManager);
		enderDragon->setPosition(glm::vec2(15 * map->getTileSize(), 5 * map->getTileSize())); // Posición inicial
	}

	powerups = {};
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
	if ((rand() % 100) < 70) {
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

void Scene::restart()
{
	lives = 3;
	loadLevel(0);
	bombManager.resetBombRange();
	player->resetNumberBombs();
}

bool Scene::canLeaveGameOver() const
{
	return (state == STATE_GAMEOVER || state == STATE_VICTORY) && stateTimer >= 1000;
}