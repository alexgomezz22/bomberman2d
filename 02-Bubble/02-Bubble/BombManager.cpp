#include "BombManager.h"

#define BOMB_RANGE 4

#define SHEET_W 128.f
#define SHEET_H 16.f
#define GROW_STEP 60   // ms que tarda el rayo en avanzar un tile

// {x, y} en píxeles de cada pieza
static const int FLAME_PIXELS[FLAME_TYPES][2] = {
	{0, 0},    // FLAME_CENTER
	{16, 0},   // FLAME_H
	{32, 0},   // FLAME_V
	{48, 0},   // FLAME_UP
	{64, 0},   // FLAME_DOWN
	{80, 0},   // FLAME_LEFT
	{96, 0}    // FLAME_RIGHT
};

BombManager::BombManager()
{
	map = NULL;
	texProgram = NULL;
	flameSprite = NULL;
}

BombManager::~BombManager()
{
	for (size_t i = 0; i < bombs.size(); i++)
		delete bombs[i];
	if (flameSprite != NULL)
		delete flameSprite;
}

void BombManager::init(const glm::ivec2& tileMapPos, TileMap* tileMap, ShaderProgram& shaderProgram)
{
	tileMapDispl = tileMapPos;
	map = tileMap;
	texProgram = &shaderProgram;

	flameSheet.loadFromFile("images/explosion.png", TEXTURE_PIXEL_FORMAT_RGBA);
	flameSprite = Sprite::createSprite(glm::ivec2(BOMB_TILE, BOMB_TILE),
		glm::vec2(16.f / SHEET_W, 16.f / SHEET_H), &flameSheet, &shaderProgram);
	flameSprite->setNumberAnimations(FLAME_TYPES);
	for (int t = 0; t < FLAME_TYPES; t++)
	{
		flameSprite->setAnimationSpeed(t, 8);
		flameSprite->addKeyframe(t, glm::vec2(FLAME_PIXELS[t][0] / SHEET_W,
			FLAME_PIXELS[t][1] / SHEET_H));
	}
}

void BombManager::addBomb(int col, int row)
{
	for (size_t i = 0; i < bombs.size(); i++)   // no dos bombas en el mismo tile
		if (bombs[i]->getState() == BOMB_FUSE && bombs[i]->getTile() == glm::ivec2(col, row))
			return;

	Bomb* b = new Bomb();
	b->init(tileMapDispl, col, row, *texProgram);
	bombs.push_back(b);
}

static void addFlame(std::vector<FlameCell>& v, int col, int row, int type, int dist)
{
	FlameCell c;
	c.tile = glm::ivec2(col, row);
	c.type = type;
	c.dist = dist;
	v.push_back(c);
}

void BombManager::detonate(Bomb* b)
{
	b->explode();
	std::vector<FlameCell>& f = b->getFlames();
	glm::ivec2 t = b->getTile();

	addFlame(f, t.x, t.y, FLAME_CENTER, 0);

	const int dx[4] = { 0, 0, -1, 1 };
	const int dy[4] = { -1, 1, 0, 0 };
	const int body[4] = { FLAME_V, FLAME_V, FLAME_H, FLAME_H };
	const int tip[4] = { FLAME_UP, FLAME_DOWN, FLAME_LEFT, FLAME_RIGHT };

	for (int d = 0; d < 4; d++)
	{
		for (int s = 1; s <= BOMB_RANGE; s++)
		{
			int c = t.x + dx[d] * s, r = t.y + dy[d] * s;
			if (map->isSolidTile(c, r))
				break;
			bool last = (s == BOMB_RANGE) || map->isSolidTile(c + dx[d], r + dy[d]);
			addFlame(f, c, r, last ? tip[d] : body[d], s);
		}
	}

	// Reacción en cadena: otras bombas tocadas por el rayo explotan ya
	for (size_t i = 0; i < bombs.size(); i++)
	{
		if (bombs[i]->getState() != BOMB_FUSE)
			continue;
		for (size_t k = 0; k < f.size(); k++)
			if (f[k].tile == bombs[i]->getTile())
			{
				detonate(bombs[i]);
				break;
			}
	}
}

void BombManager::update(int deltaTime)
{
	for (size_t i = 0; i < bombs.size(); i++)
		bombs[i]->update(deltaTime);

	for (size_t i = 0; i < bombs.size(); i++)
		if (bombs[i]->fuseFinished())
			detonate(bombs[i]);

	for (size_t i = 0; i < bombs.size(); )   // borrar las terminadas
	{
		if (bombs[i]->getState() == BOMB_DONE)
		{
			delete bombs[i];
			bombs.erase(bombs.begin() + i);
		}
		else
			i++;
	}
}

void BombManager::render()
{
	for (size_t i = 0; i < bombs.size(); i++)
	{
		bombs[i]->render();
		if (bombs[i]->getState() != BOMB_EXPLODING)
			continue;
		std::vector<FlameCell>& f = bombs[i]->getFlames();
		for (size_t k = 0; k < f.size(); k++)
		{
			if (f[k].dist * GROW_STEP > bombs[i]->getTimer()) continue;
			flameSprite->changeAnimation(f[k].type);
			flameSprite->setPosition(glm::vec2(float(tileMapDispl.x + f[k].tile.x * BOMB_TILE),
				float(tileMapDispl.y + f[k].tile.y * BOMB_TILE)));
			flameSprite->render();
		}
	}
}

bool BombManager::isInExplosion(const glm::ivec2& pos, const glm::ivec2& size) const
{
	for (size_t i = 0; i < bombs.size(); i++)
	{
		if (bombs[i]->getState() != BOMB_EXPLODING)
			continue;
		std::vector<FlameCell>& f = bombs[i]->getFlames();
		for (size_t k = 0; k < f.size(); k++)
		{
			if (f[k].dist * GROW_STEP > bombs[i]->getTimer()) continue;
			int fx = f[k].tile.x * BOMB_TILE, fy = f[k].tile.y * BOMB_TILE;
			if (pos.x < fx + BOMB_TILE && pos.x + size.x > fx &&
				pos.y < fy + BOMB_TILE && pos.y + size.y > fy)
				return true;
		}
	}
	return false;
}

void BombManager::clear()
{
	for (size_t i = 0; i < bombs.size(); i++)
		delete bombs[i];
	bombs.clear();
}