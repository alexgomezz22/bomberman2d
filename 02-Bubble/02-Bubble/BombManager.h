#ifndef _BOMB_MANAGER_INCLUDE
#define _BOMB_MANAGER_INCLUDE

#include <vector>
#include "Bomb.h"
#include "TileMap.h"

// Forward declaration
class Scene;

class BombManager
{
public:
	BombManager();
	~BombManager();

	void init(const glm::ivec2& tileMapPos, TileMap* tileMap, ShaderProgram& shaderProgram, int mapSize_x, Scene* scenePtr);
	void update(int deltaTime);
	void render();

	std::vector<Bomb*>& getBombs() { return bombs; }
	bool isInExplosion(const glm::ivec2& pos, const glm::ivec2& size) const;
	void clear();
	bool addBomb(int col, int row);
	int countActive() const;
	void setMap(TileMap* m) { map = m; }

private:
	void detonate(Bomb* b);
	std::vector<Bomb*> bombs;
	TileMap* map;
	ShaderProgram* texProgram;
	glm::ivec2 tileMapDispl;
	Texture flameSheet;
	Sprite* flameSprite;
	int mapSizeX;
	Scene* scene;
};

#endif