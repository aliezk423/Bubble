#pragma once
#include "cocos2d.h"
#include "player.h"

USING_NS_CC;

class player;
class MapLayer : public Layer
{
public:
	static Scene *createScene();
	std::vector<std::vector<int>> init_prop_mp();
	virtual bool init();
	CREATE_FUNC(MapLayer);
	//void update(float);
	void b1ToOne(float);
	void b2ToOne(float);
	void bombExplosion(int id);
	bool isCollid(Vec2, Vec2);
	int isDestroy(Vec2 pos);
	Vec2 tmxFromPos(Vec2);
	void RemoveExplosion(float dt);
	void update(float dt);
	void buffAndRemove(int id);
	Vec2 toInt(Vec2 pos);
	std::vector<std::vector<int>> props;
	void killPlayer(Vec2 pos[], int ct);
	void revealProps(int x, int y, int zorder);
	bool BombExist(Vec2 pos);

private:
	Size vsable = {750, 750};
	bool b1 = 1;
	bool b2 = 1;
	double ss = 0;
	player *playerAll[2];
	Sprite *playAll[2];
	TMXTiledMap *_map;
	int zorder[15] = {0};
	int TagOrder = 999;
	std::vector<int> BombTag;
	std::vector<int> ExplosionTag;
	std::map<Vec2, int> propsTag; //道具列表
	int propCount = 0;
	std::map<Vec2, int> barriersTag; //障碍物列表
	TMXLayer *collid;
	TMXLayer *barrier0;
	bool isWin = 0;
};
