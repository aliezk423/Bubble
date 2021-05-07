#include "MapLayer.h"
#include "SimpleAudioEngine.h"
#include "EndLayer.h"

//重载createScene，加载场景，在CREATEFUNC中调用
Scene *MapLayer::createScene()
{
	Scene *scene = Scene::create();
	MapLayer *map = MapLayer::create();
	scene->addChild(map);
	return scene;
}

//初始化道具位置
//@param 障碍物地图
//@return 道具地图
std::vector<std::vector<int>> MapLayer::init_prop_mp()
{
	std::vector<std::vector<int>> prop_mp; //初始化二维数组
	int width = 15;
	int height = 15;
	for (int i = 0; i < width; i++) //遍历数组
	{
		std::vector<int> col;
		for (int j = 0; j < height; j++)
		{
			auto mp = barrier0->getTileGIDAt(Vec2(i, j));
			if (mp == 6 || mp == 7 || mp == 8) //积木与木箱可炸
			{
				col.push_back(0);
			}
			else
			{
				col.push_back(-1);
			}
		}
		prop_mp.push_back(col);
	}
	//未放置道具前：  0:随机初始化为道具，-1:无任何效果
	//放置道具后：   0:无，1:炸弹变长，2:炸弹增多，3:生命+1

	std::vector<Vec2> V1; //存储右上角能生成道具的点
	std::vector<Vec2> V2; //存储左下角能生成道具的点

	//15*15的格子分成4部分判断
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{

			if (prop_mp[i][j] == -1) //无道具生成点跳过
			{
				continue;
			}

			if (i < 7 && j >= 7) //右上，存在保底数量
			{
				V1.push_back(Vec2(i, j));
			}

			if (i >= 7 && j < 7) //左下，存在保底数量
			{
				V2.push_back(Vec2(i, j));
			}
			if ((i <= 6 && j <= 6) || (i >= 7 && j >= 7)) //左上和右下，随机生成一定数量
			{
				int tmp = rand() % 10;
				if (tmp < 5)
				{
					prop_mp[i][j] = 0; //50%无道具
				}
				else
				{
					prop_mp[i][j] = rand() % 3 + 1; //50%有道具
				}
			}
		}
	}
	prop_mp[5][5] = rand() % 3 + 1;	 //左上角100%道具生成点
	prop_mp[8][10] = rand() % 3 + 1; //右下角100%道具生成点
	bool isV1[100];					 //是否已存在道具
	memset(isV1, 0, sizeof(isV1));
	int cnt1 = 5; //左下角随机放5个
	while (cnt1 > 0)
	{
		int tmp = rand() % V1.size(); //随机生成
		if (!isV1[tmp])				  //该位置没有道具
		{
			isV1[tmp] = 1; //标记该位置
			cnt1--;
		}
	}
	for (int i = 0; i < V1.size(); ++i) //遍历左下角方块
	{
		if (isV1[i]) //该位置被标记
		{
			prop_mp[V1[i].x][V1[i].y] = rand() % 3 + 1; //放置道具
		}
	}
	//右下角随机放5个，具体实现同上
	bool isV2[100];
	memset(isV2, 0, sizeof(isV2));
	int cnt2 = 5;
	while (cnt2 > 0)
	{
		int tmp = rand() % V2.size();
		if (!isV2[tmp])
		{
			isV2[tmp] = 1;
			cnt2--;
		}
	}
	for (int i = 0; i < V2.size(); ++i)
	{
		if (isV2[i])
		{
			prop_mp[V2[i].x][V2[i].y] = rand() % 3 + 1;
		}
	}
	return prop_mp; //返回道具生成地图
}

//初始化场景，完成实例化，在CREATEFUNC中调用
bool MapLayer::init()
{
	if (!Layer::init())
		return false; //创建失败

	/*播放bgm*/
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("music/bgm2.mp3", true);

	/*初始化变量*/
	srand(time(NULL));
	BombTag = std::vector<int>();
	ExplosionTag = std::vector<int>();

	/*读取并分解地图，创建人物，加载界面*/
	TMXTiledMap *map = TMXTiledMap::create("map.tmx"); //导入tmx地图
	_map = map;										   //存储tmx地图类
	//   ss : 缩放系数，即该场景精灵的缩放比例
	ss = Director::sharedDirector()->getVisibleSize().height / _map->getContentSize().height;
	_map->setScale(ss);
	_map->setAnchorPoint(Vec2(0.5f, 0.5f));					//设置锚点为场景中心
	_map->setPosition(vsable.width / 2, vsable.height / 2); //设置位置为场景中心，以便铺满场景
	addChild(_map, 0);										//向场景载入tmx地图
	collid = _map->getLayer("collid");						//读取tmx地图中的碰撞层
	auto barrier = _map->getLayer("barrier");				//读取tmx地图中的障碍物层
	barrier0 = _map->getLayer("barrier0");					//读取tmx地图中的障碍物区分层
	props = init_prop_mp();									//生成道具地图
	//分解障碍物层，将该层的瓦片分离并加入场景，使之与人物精灵同级
	for (int j = 0; j < 15; j++)
	{
		for (int i = 0; i < 15; i++)
		{
			if (barrier->getTileGIDAt(Vec2(i, j)) > 0)
			{											  //如果（i，j）上有瓦片
				auto sp = barrier->getTileAt(Vec2(i, j)); //获取瓦片精灵
				auto pos = sp->getPosition();			  //获取瓦片实际位置
				barrier->removeTileAt(Vec2(i, j));		  //将障碍物中该瓦片移除（不移除无法重新载入场景）
				sp->setScale(ss);
				sp->setAnchorPoint(Vec2(0, 0));
				sp->setPosition(pos * ss);
				auto p = pos * ss;
				addChild(sp, j * 15 + i); //向场景载入(i,j)位置的瓦片，并设置ZOrder
				barriersTag[tmxFromPos(p)] = TagOrder;
				sp->setTag(TagOrder++);
				/*sp->getTexture()->setAntiAliasTexParameters();//开启抗锯齿*/
				zorder[j] = j * 15 + i; //将同一行最大的ZOrder值存入zorder数组
			}
		}
	}
	//加载人物精灵，并赋予tmx地图中对象层的属性
	//人物尺寸为40*54
	auto play = _map->getObjectGroup("ren"); //读取人物对象层
	auto one = play->getObject("player1");	 //读取对象player1
	double x = one["x"].asDouble();
	double y = one["y"].asDouble();			  //读取player1的位置属性
	auto play1 = Sprite::create("Role1.png"); //创建人物精灵play1
	play1->setAnchorPoint(Vec2(0, 0));
	play1->setPosition(toInt(Vec2(x * ss, y * ss))); //赋予play1，对象player1的位置属性
	play1->setScale(ss);
	this->addChild(play1, zorder[9] + 1); //向场景载入play1，并设置初始ZOrder值
	//play1->getTexture()->setAntiAliasTexParameters();
	auto player1 = new player("jack"); //创建play1人物属性类
	playerAll[0] = player1;
	playAll[0] = play1;					   //添加关联
	auto two = play->getObject("player2"); //读取对象player2
	x = two["x"].asDouble();
	y = two["y"].asDouble();				  //读取player2的位置属性
	auto play2 = Sprite::create("Role2.png"); //创建人物精灵play2
	play2->setAnchorPoint(Vec2(0, 0));
	play2->setPosition(toInt(Vec2(x * ss, y * ss))); //赋予play2，对象player2的位置属性
	play2->setScale(ss);
	this->addChild(play2, zorder[1] + 1); //向场景载入play2，并设置初始ZOrder值
	//play2->getTexture()->setAntiAliasTexParameters();
	auto player2 = new player("rose"); //创建play2人物属性类
	playerAll[1] = player2;
	playAll[1] = play2; //添加关联

	/*设置键盘监听，使人物可以移动、放置炸弹*/
	auto listenerKey = EventListenerKeyboard::create();							//创建按键监听器
	const float speed = 0.3f;													//设置移动速度及动画播放速度
	listenerKey->onKeyPressed = ([=](EventKeyboard::KeyCode code, Event *event) //按键被按下事件，值为匿名函数（[=]为传递父类中变量）
	{
		if (b1)
		{ //W、A、S、D被按下
			switch (code)
			{
			case EventKeyboard::KeyCode::KEY_W:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0)); //初始化人物移动
				if (isCollid(Vec2(0, 50), play1->getPosition()))
				{ //上方的碰撞检测
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(0, 50)); //向上移动一格，50=屏幕大小/格子数
				}
				auto pAmiFrame = Vector<SpriteFrame *>(); //初始化帧动画数组
				b1 = 0;								   //禁止W、A、S、D短期内再按下

				//存入动画帧
				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role1action.png", Rect(j * 39, 0, 38, 52));
					//39=图片宽度-1像素边缘，38=图片宽度-左右两侧的1像素边缘，52=图片高度-上下两侧的1像素边缘
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role1action.png", Rect(0, 0, 38, 52)));

				//创建动画
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto spawn = Spawn::create(animate, moveBy, NULL); //使人物的动画和移动同时进行

				play1->runAction(spawn); //载入人物动画
				break;
			}
			case EventKeyboard::KeyCode::KEY_A:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0));
				if (isCollid(Vec2(-50, 0), play1->getPosition()))
				{
				} //左方的碰撞检测
				else
				{
					moveBy = MoveBy::create(speed, Vec2(-50, 0)); //向左移动一格
				}
				auto pAmiFrame = Vector<SpriteFrame *>();
				b1 = 0;

				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role1action.png", Rect(j * 39, 2 * 53, 38, 52));
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role1action.png", Rect(0, 2 * 53, 38, 52)));
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL);
				auto repeat = RepeatForever::create(seq);
				auto spawn = Spawn::create(animate, moveBy, NULL);
				play1->runAction(spawn);
				break;
			}
			case EventKeyboard::KeyCode::KEY_S:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0));
				if (isCollid(Vec2(0, -50), play1->getPosition()))
				{ //下方的碰撞检测
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(0, -50)); //向下移动一格
				}
				auto pAmiFrame = Vector<SpriteFrame *>();
				b1 = 0;
				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role1action.png", Rect(j * 39, 1 * 53, 38, 52));
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role1action.png", Rect(0, 1 * 53, 38, 52)));
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL);
				auto repeat = RepeatForever::create(seq);
				auto spawn = Spawn::create(animate, moveBy, NULL);
				play1->runAction(spawn);
				break;
			}
			case EventKeyboard::KeyCode::KEY_D:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0));
				if (isCollid(Vec2(50, 0), play1->getPosition()))
				{ //右方的碰撞检测
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(50, 0)); //向右移动一格
				}
				auto pAmiFrame = Vector<SpriteFrame *>();
				b1 = 0;
				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role1action.png", Rect(j * 39, 3 * 53, 38, 52));
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role1action.png", Rect(0, 3 * 53, 38, 52)));
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL);
				auto repeat = RepeatForever::create(seq);
				auto spawn = Spawn::create(animate, moveBy, NULL);
				play1->runAction(spawn);
				break;
			}
			default:
				break;
			}
			auto p = tmxFromPos(play1->getPosition());						  //读取play1的tmx地图坐标
			this->reorderChild(play1, zorder[(int)p.y] + 1);				  //改变play1的ZOrder，实现障碍物与人物之间的遮挡关系
			this->scheduleOnce(schedule_selector(MapLayer::b1ToOne), 0.25f); //延时0.25秒使 b1 = 1
		}
		if (b2)
		{ //上、下、左、右被按下，具体实现同上
			switch (code)
			{
			case EventKeyboard::KeyCode::KEY_UP_ARROW:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0));
				if (isCollid(Vec2(0, 50), play2->getPosition()))
				{
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(0, 50));
				}
				auto pAmiFrame = Vector<SpriteFrame *>();
				b2 = 0;
				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role2action.png", Rect(j * 39, 0, 38, 52));
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role2action.png", Rect(0, 0, 38, 52)));
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL);
				auto repeat = RepeatForever::create(seq);
				auto spawn = Spawn::create(animate, moveBy, NULL);
				play2->runAction(spawn);
				break;
			}
			case EventKeyboard::KeyCode::KEY_LEFT_ARROW:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0));
				if (isCollid(Vec2(-50, 0), play2->getPosition()))
				{
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(-50, 0));
				}
				auto pAmiFrame = Vector<SpriteFrame *>();
				b2 = 0;
				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role2action.png", Rect(j * 39, 2 * 53, 38, 52));
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role2action.png", Rect(0, 2 * 53, 38, 52)));
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL);
				auto repeat = RepeatForever::create(seq);
				auto spawn = Spawn::create(animate, moveBy, NULL);
				play2->runAction(spawn);
				break;
			}
			case EventKeyboard::KeyCode::KEY_DOWN_ARROW:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0));
				if (isCollid(Vec2(0, -50), play2->getPosition()))
				{
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(0, -50));
				}
				auto pAmiFrame = Vector<SpriteFrame *>();
				b2 = 0;
				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role2action.png", Rect(j * 39, 1 * 53, 38, 52));
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role2action.png", Rect(0, 1 * 53, 38, 52)));
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL);
				auto repeat = RepeatForever::create(seq);
				auto spawn = Spawn::create(animate, moveBy, NULL);
				play2->runAction(spawn);
				break;
			}
			case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0));
				if (isCollid(Vec2(50, 0), play2->getPosition()))
				{
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(50, 0));
				}
				auto pAmiFrame = Vector<SpriteFrame *>();
				b2 = 0;
				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role2action.png", Rect(j * 39, 3 * 53, 38, 52));
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role2action.png", Rect(0, 3 * 53, 38, 52)));
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL);
				auto repeat = RepeatForever::create(seq);
				auto spawn = Spawn::create(animate, moveBy, NULL);
				play2->runAction(spawn);
				break;
			}
			default:
				break;
			}
			auto p = tmxFromPos(play2->getPosition());
			this->reorderChild(play2, zorder[(int)p.y] + 1);
			this->scheduleOnce(schedule_selector(MapLayer::b2ToOne), 0.2f); //延时
		}
		switch (code)
		{
		case EventKeyboard::KeyCode::KEY_SPACE: //空格键被按下
		{
			if (player1->getBombNum() > 0) //按照可使用炸弹数量，放置炸弹
			{
				auto pos = play1->getPosition();
				int x = round(pos.x); //x取整
				int y = round(pos.y); //y取整
				int xs = x % 50;
				int ys = y % 50;										//人物距离格子偏移量
				auto position = play1->getPosition() + Vec2(-xs, -ys); //炸弹放置位置
				if (BombExist(toInt(position)))
				{ //当前位置已存在炸弹，跳过
					break;
				}
				auto bomb = Sprite::create("Popo.png", Rect(0, 0, 38, 38)); //创建炸弹，38=图片宽度-左右两侧的1像素边缘
				bomb->setPosition(toInt(position));						 //在人物附近的格子放置炸弹
				bomb->setScale(ss);
				bomb->setAnchorPoint(Vec2(0, 0));
				addChild(bomb, play1->getLocalZOrder() - 1); //向场景中载入炸弹，并设置ZOrder
				bomb->setTag(TagOrder);					  //设置炸弹精灵的标签，以便从场景中调用
				BombTag.push_back(TagOrder++);				  //存储当前炸弹标签，TagOrder值加一
				player1->setBomb();						  //放置炸弹
				auto pAmi = Vector<SpriteFrame *>();		  //初始化帧动画数组

				//存入动画帧
				for (int i = 0; i < 3; i++)
				{
					auto p = SpriteFrame::create("Popo.png", Rect(i * 39, 0, 38, 38));
					pAmi.pushBack(p);
				}

				//创建动画
				auto animation = Animation::createWithSpriteFrames(pAmi, 0.2f);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL); //创建序列
				auto repeat = Repeat::create(seq, 3);						   //使动画重复播放3次

				auto fun = CallFunc::create([=]() { //回调函数，使炸弹爆炸
					this->bombExplosion(1);
				});
				auto rf = Sequence::create(repeat, fun, NULL); //合成先放置炸弹，后爆炸的动画
				bomb->runAction(rf);							//载入炸弹动画
			}
			break;
		}
		case EventKeyboard::KeyCode::KEY_ENTER: //回车键被按下，具体实现同上
		{
			if (player2->getBombNum() > 0)
			{
				auto pos = play2->getPosition(); //屏幕坐标误差修正， x , y 均近似整数
				int x = round(pos.x);
				int y = round(pos.y);
				int xs = x % 50;
				int ys = y % 50;
				auto position = play2->getPosition() + Vec2(-xs, -ys);
				if (BombExist(toInt(position)))
				{
					break;
				}
				auto bomb = Sprite::create("Popo.png", Rect(0, 0, 38, 38));
				bomb->setPosition(toInt(position));
				bomb->setScale(ss);
				bomb->setAnchorPoint(Vec2(0, 0));
				addChild(bomb, play2->getLocalZOrder() - 1);
				bomb->setTag(TagOrder);
				BombTag.push_back(TagOrder++);
				player2->setBomb();
				auto pAmi = Vector<SpriteFrame *>();
				for (int i = 0; i < 3; i++)
				{
					auto p = SpriteFrame::create("Popo.png", Rect(i * 39, 0, 38, 38));
					pAmi.pushBack(p);
				}
				auto animation = Animation::createWithSpriteFrames(pAmi, 0.2f);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL);
				auto repeat = Repeat::create(seq, 3);

				auto fun = CallFunc::create([=]() {
					this->bombExplosion(2);
				});
				auto rf = Sequence::create(repeat, fun, NULL);
				bomb->runAction(rf);
			}
			break;
		}
		default:
			break;
		}
		//log("%.1f,%.1f", play1->getPositionX(), play1->getPositionY());
	});
	auto dispatcher = Director::getInstance()->getEventDispatcher(); //创建事件分发器
	//添加按键监听，到事件分发器
	dispatcher->addEventListenerWithSceneGraphPriority(listenerKey, this);
	this->scheduleUpdate(); //开启帧定时器
	return true;
}

void MapLayer::b1ToOne(float dt)
{
	b1 = 1;
}

void MapLayer::b2ToOne(float dt)
{
	b2 = 1;
}

//炸弹引爆函数，消除炸弹，并添加爆炸精灵
//@param 人物id
void MapLayer::bombExplosion(int id)
{
	if (BombTag.empty()) //场景中没有炸弹
		return;
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("music/boom.mp3");
	player *p = playerAll[id - 1];
	auto bomb = (Sprite *)getChildByTag(BombTag.front()); //取出BombTag数组首部的标签号
	Vec2 pos = bomb->getPosition();						  //保存炸弹位置
	log("check:%f,%f", pos.x, pos.y);
	int z = bomb->getLocalZOrder();	   //保存炸弹ZOrder
	removeChildByTag(BombTag.front()); //从场景中移除炸弹精灵
	BombTag.erase(BombTag.begin());	   //移除BombTag数组的炸弹标签

	//中心爆炸效果
	auto center = Sprite::create("Explosion.png", Rect(0, 4 * 39, 40, 40)); //创建中心爆炸精灵
	center->setPosition(pos);
	center->setScale(ss);
	center->setAnchorPoint(Vec2(0, 0));
	addChild(center, z);				 //向场景载入中心爆炸精灵
	center->setTag(TagOrder);			 //设置爆炸精灵的标签，以便从场景中调用
	ExplosionTag.push_back(TagOrder++);	 //存储当前爆炸精灵标签，TagOrder值加一
	auto pAmi = Vector<SpriteFrame *>(); //初始化帧动画数组

	//存入动画帧
	for (int i = 0; i < 8; i++)
	{
		auto p = SpriteFrame::create("Explosion.png", Rect(i % 4 * 39, 4 * 39, 40, 40));
		pAmi.pushBack(p);
	}
	pAmi.pushBack(SpriteFrame::create("Explosion.png", Rect(4 * 40, 4 * 40, 38, 38)));
	//存入一张空白的贴图，作为爆炸效果的最终状态

	//创建动画
	auto animation = Animation::createWithSpriteFrames(pAmi, 0.07f);
	auto animate = Animate::create(animation);
	center->runAction(animate); //载入中心爆炸动画

	//上方
	for (int j = 1; j <= p->getBombLength(); j++)
	{												   //按照炸弹威力生成爆炸精灵
		auto check = isDestroy(pos + Vec2(0, 50 * j)); //判断爆炸对地形造成的影响
		if (check == 2)
			break; //边界或房子，树木阻挡爆炸
		auto front = Sprite::create("Explosion.png", Rect(0, 0, 40, 40));
		front->setPosition(pos + Vec2(0, 50 * j)); //使上方爆炸精灵位于原炸弹上方
		front->setScale(ss);
		front->setAnchorPoint(Vec2(0, 0));
		addChild(front, z);
		front->setTag(TagOrder);
		ExplosionTag.push_back(TagOrder++);
		pAmi = Vector<SpriteFrame *>();
		for (int i = 0; i < 14; i++)
		{
			auto p = SpriteFrame::create("Explosion.png", Rect(i * 39, 0, 40, 40));
			pAmi.pushBack(p);
		}
		pAmi.pushBack(SpriteFrame::create("Explosion.png", Rect(4 * 40, 4 * 40, 38, 38)));
		animation = Animation::createWithSpriteFrames(pAmi, 0.07f);
		animate = Animate::create(animation);
		front->runAction(animate);
		if (check == 1)
		{ //积木，箱子被爆炸销毁
			auto pos_ = pos + Vec2(0, 50 * j);
			removeChildByTag(barriersTag[tmxFromPos(pos_)]);
			auto pso = tmxFromPos(pos_);
			collid->removeTileAt(pso);
			barrier0->removeTileAt(pso);
			revealProps(pos_.x, pos_.y, zorder[(int)pso.y]);
			break;
		}
	}

	//下方，具体实现同上
	for (int j = 1; j <= p->getBombLength(); j++)
	{
		auto check = isDestroy(pos + Vec2(0, -50 * j));
		if (check == 2)
			break;
		auto down = Sprite::create("Explosion.png", Rect(0, 39, 40, 40));
		down->setPosition(pos + Vec2(0, -50 * j)); //使下方爆炸精灵位于原炸弹下方
		down->setScale(ss);
		down->setAnchorPoint(Vec2(0, 0));
		addChild(down, z);
		down->setTag(TagOrder);
		ExplosionTag.push_back(TagOrder++);
		pAmi = Vector<SpriteFrame *>();
		for (int i = 0; i < 14; i++)
		{
			auto p = SpriteFrame::create("Explosion.png", Rect(i * 39, 39, 40, 40));
			pAmi.pushBack(p);
		}
		pAmi.pushBack(SpriteFrame::create("Explosion.png", Rect(4 * 40, 4 * 40, 38, 38)));
		animation = Animation::createWithSpriteFrames(pAmi, 0.07f);
		animate = Animate::create(animation);
		down->runAction(animate);
		if (check == 1)
		{
			auto pos_ = pos + Vec2(0, -50 * j);
			removeChildByTag(barriersTag[tmxFromPos(pos_)]);
			auto pso = tmxFromPos(pos_);
			collid->removeTileAt(pso);
			barrier0->removeTileAt(pso);
			revealProps(pos_.x, pos_.y, zorder[(int)pso.y]);
			break;
		}
	}

	//左方，具体实现同上
	for (int j = 1; j <= p->getBombLength(); j++)
	{
		auto check = isDestroy(pos + Vec2(-50 * j, 0));
		if (check == 2)
			break;
		auto left = Sprite::create("Explosion.png", Rect(0, 2 * 39, 40, 40));
		left->setPosition(pos + Vec2(-50 * j, 0)); //使左方爆炸精灵位于原炸弹左方
		left->setScale(ss);
		left->setAnchorPoint(Vec2(0, 0));
		addChild(left, z);
		left->setTag(TagOrder);
		ExplosionTag.push_back(TagOrder++);
		pAmi = Vector<SpriteFrame *>();
		for (int i = 0; i < 14; i++)
		{
			auto p = SpriteFrame::create("Explosion.png", Rect(i * 39, 2 * 39, 40, 40));
			pAmi.pushBack(p);
		}
		pAmi.pushBack(SpriteFrame::create("Explosion.png", Rect(4 * 40, 4 * 40, 38, 38)));
		animation = Animation::createWithSpriteFrames(pAmi, 0.07f);
		animate = Animate::create(animation);
		left->runAction(animate);
		if (check == 1)
		{
			auto pos_ = pos + Vec2(-50 * j, 0);
			removeChildByTag(barriersTag[tmxFromPos(pos_)]);
			auto pso = tmxFromPos(pos_);
			collid->removeTileAt(pso);
			barrier0->removeTileAt(pso);
			revealProps(pos_.x, pos_.y, zorder[(int)pso.y]);
			break;
		}
	}

	//右方，具体实现同上
	for (int j = 1; j <= p->getBombLength(); j++)
	{
		auto check = isDestroy(pos + Vec2(50 * j, 0));
		if (check == 2)
			break;
		auto right = Sprite::create("Explosion.png", Rect(0, 3 * 39, 40, 40));
		right->setPosition(pos + Vec2(50 * j, 0)); //使右方爆炸精灵位于原炸弹右方
		right->setScale(ss);
		right->setAnchorPoint(Vec2(0, 0));
		addChild(right, z);
		right->setTag(TagOrder);
		ExplosionTag.push_back(TagOrder++);
		pAmi = Vector<SpriteFrame *>();
		for (int i = 0; i < 14; i++)
		{
			auto p = SpriteFrame::create("Explosion.png", Rect(i * 39, 3 * 39, 40, 40));
			pAmi.pushBack(p);
		}
		pAmi.pushBack(SpriteFrame::create("Explosion.png", Rect(4 * 40, 4 * 40, 38, 38)));
		animation = Animation::createWithSpriteFrames(pAmi, 0.07f);
		animate = Animate::create(animation);
		right->runAction(animate);
		if (check == 1)
		{
			auto pos_ = pos + Vec2(50 * j, 0);
			removeChildByTag(barriersTag[tmxFromPos(pos_)]);
			auto pso = tmxFromPos(pos_);
			collid->removeTileAt(pso);
			barrier0->removeTileAt(pso);
			revealProps(pos_.x, pos_.y, zorder[(int)pso.y]);
			break;
		}
	}
	p->incBombNum(); //爆炸完毕，可使用炸弹数量增加

	//延时0.9s，即动画播放后，移除爆炸精灵
	scheduleOnce(schedule_selector(MapLayer::RemoveExplosion), 0.9f);
}

//碰撞检测函数
//@param 位移量，原位置
//@return false未碰撞，true发生碰撞
bool MapLayer::isCollid(Vec2 npos, Vec2 mpos)
{
	Vec2 pos = (npos + mpos);									//位移后位置
	Vec2 tile = tmxFromPos(pos);								//获取pos所对应的tmx地图坐标
	if (tile.x < 0 || tile.x > 14 || tile.y < 0 || tile.y > 14) //边界检测
		return true;
	int tileGid = collid->getTileGIDAt(tile); //获取tile所对应的瓦片GID
	if (tileGid > 0)
	{													 //tile位置有障碍物
		Value prop = _map->getPropertiesForGID(tileGid); //获取瓦片的属性
		ValueMap propMap = prop.asValueMap();			 //将属性转换成Map
		bool colid = propMap["collid"].asBool();		 //读取瓦片collid属性的值
		if (colid)
		{ //collid为真
			//碰撞特效
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("music/wall.mp3");
			return true;
		}
	}
	return false;
}

//炸弹破坏检测函数
//@param 检测位置
//@return 0:无遮挡，1:可破坏，2:边界或不可破坏
int MapLayer::isDestroy(Vec2 pos)
{
	Vec2 tile = tmxFromPos(pos);								//获取pos所对应的tmx地图坐标
	if (tile.x < 0 || tile.x > 14 || tile.y < 0 || tile.y > 14) //边界检测
		return 2;
	int tileGid = barrier0->getTileGIDAt(tile); //获取tile所对应的瓦片GID
	switch (tileGid)
	{
	case 1: //1.树木
	case 2:
	case 3:
	case 4: //2.3.4.房子
		return 2;
		break;
	case 6: //6.箱子
	case 7:
	case 8: //7.8.积木
		return 1;
		break;
	}
	return 0;
}

//屏幕坐标->tmx地图坐标
Vec2 MapLayer::tmxFromPos(Vec2 pos)
{
	//Vec2 pos_ = pos + Vec2(2.5, -2.5);//屏幕坐标误差修正， x , y 均近似整数
	int x = round(pos.x / 50);		//四舍五入计算x坐标
	int y = 14 - round(pos.y / 50); //四舍五入计算y坐标，并翻转坐标系
	return Vec2(x, y);
}

//移除爆炸精灵
//@param dt无实际效果，为定时器所使用，不可缺少
void MapLayer::RemoveExplosion(float dt)
{
	Vec2 poss[100];
	int ct = 0;
	for (int i = 0; i < ExplosionTag.size(); i++)
	{ //遍历爆炸标签数组
		poss[ct++] = getChildByTag(ExplosionTag[i])->getPosition();
		removeChildByTag(ExplosionTag[i]); //移除场景中标签所对应的精灵
	}
	ExplosionTag.clear(); //清空ExplosionTag数组
	killPlayer(poss, ct);
}

//帧定时器更新函数
void MapLayer::update(float dt)
{
	for (int i = 0; i < 2; ++i)
	{
		auto pos = toInt(playAll[i]->getPosition());
		int x = pos.x;
		int y = pos.y;
		if (x % 50 == 0 && y % 50 == 0)
		{ //人物静止时，判断道具是否被拾起
			buffAndRemove(i + 1);
		}
	}
}

//击杀玩家函数
//@param 爆炸位置
void MapLayer::killPlayer(Vec2 pos[], int ct)
{
	if (isWin)
		return;
	std::string endString = "";
	for (int i = 0; i < 2; i++)
	{
		auto play = playAll[i];
		auto player = playerAll[i];
		auto p = play->getPosition();
		for (int j = 0; j < ct; ++j)
		{
			if (pos[j] == p)
			{
				player->decHealth();
			}
		}
	}
	if (playerAll[0]->getHealth() <= 0 && playerAll[1]->getHealth() <= 0)
	{
		isWin = 1;
		endString = "好可惜呀~，平局呢！";
		//MessageBoxW(nullptr, L"可惜，平局哦！", L"Game Over", MB_OK);
		log("平局");
	}
	else
	{
		if (playerAll[0]->getHealth() <= 0)
		{
			isWin = 1;
			endString = "恭喜，Player2赢了哦！";
			//MessageBoxW(nullptr, L"Player2 Win!!!", L"Game Over", MB_OK);
			log("Player2 Win!!!");
		}
		else if (playerAll[1]->getHealth() <= 0)
		{
			isWin = 1;
			endString = "恭喜，Player1赢了哦！";
			//MessageBoxW(nullptr, L"Player1 Win!!!", L"Game Over", MB_OK);
			log("Player1 Win!!!");
		}
	}
	if (endString != "")
	{
		str = endString;
		CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
		Director::sharedDirector()->replaceScene(EndLayer::createScene());
	}
}

//显示道具
//@param 屏幕x坐标,屏幕y坐标，ZOrder
void MapLayer::revealProps(int nx, int ny, int zorder)
{
	auto pos = tmxFromPos(Vec2(nx, ny)); //转换地图坐标
	int x = pos.x;
	int y = pos.y;
	std::string t;
	if (props[y][x] == 1)
		t = "GiftPower.png";
	else if (props[y][x] == 2)
		t = "GiftPoPo.png";
	else if (props[y][x] == 3)
		t = "heart.png";
	else
		return;
	auto prop = Sprite::create(t, Rect(0, 0, 38, 38));
	prop->setPosition(Vec2(nx, ny)); //放置道具
	prop->setScale(ss);
	prop->setAnchorPoint(Vec2(0, 0));
	addChild(prop, zorder - 15); //向场景中载入道具，并设置ZOrder
	propsTag[toInt(Vec2(nx, ny))] = TagOrder;
	prop->setTag(TagOrder++);
	++propCount;
	auto pAmi = Vector<SpriteFrame *>(); //初始化帧动画数组
	//存入动画帧
	for (int i = 0; i < 3; i++)
	{
		auto p = SpriteFrame::create(t, Rect(i * 39, 0, 38, 38));
		pAmi.pushBack(p);
	}

	//创建动画
	auto animation = Animation::createWithSpriteFrames(pAmi, 0.2f);
	auto animate = Animate::create(animation);
	auto seq = (ActionInterval *)Sequence::create(animate, NULL); //创建序列
	auto repeat = RepeatForever::create(seq);
	prop->runAction(repeat); //载入动画
}

bool MapLayer::BombExist(Vec2 pos)
{
	for (int i = 0; i < BombTag.size(); i++)
	{
		auto bomb = getChildByTag(BombTag[i]);
		auto p = bomb->getPosition();
		if (p == pos)
		{
			return true;
		}
	}
	return false;
}

void MapLayer::buffAndRemove(int id)
{
	auto p = playerAll[id - 1];
	auto player = playAll[id - 1];
	auto pos = toInt(player->getPosition());
	auto pos_ = tmxFromPos(pos);
	switch (props[pos_.y][pos_.x])
	{
	case -1:
	case 0:
		return;
		break;
	case 1:
		p->incBombLength();
		break;
	case 2:
		p->incBombNum();
		break;
	case 3:
		p->incHealth();
		break;
	}
	props[pos_.y][pos_.x] = 0;
	removeChildByTag(propsTag[pos]);
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("music/get.mp3");
	--propCount;
}

Vec2 MapLayer::toInt(Vec2 pos)
{
	int x = round(pos.x);
	int y = round(pos.y);
	return Vec2(x, y);
}
