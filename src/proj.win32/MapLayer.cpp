#include "MapLayer.h"
#include "SimpleAudioEngine.h"
#include "EndLayer.h"

//����createScene�����س�������CREATEFUNC�е���
Scene *MapLayer::createScene()
{
	Scene *scene = Scene::create();
	MapLayer *map = MapLayer::create();
	scene->addChild(map);
	return scene;
}

//��ʼ������λ��
//@param �ϰ����ͼ
//@return ���ߵ�ͼ
std::vector<std::vector<int>> MapLayer::init_prop_mp()
{
	std::vector<std::vector<int>> prop_mp; //��ʼ����ά����
	int width = 15;
	int height = 15;
	for (int i = 0; i < width; i++) //��������
	{
		std::vector<int> col;
		for (int j = 0; j < height; j++)
		{
			auto mp = barrier0->getTileGIDAt(Vec2(i, j));
			if (mp == 6 || mp == 7 || mp == 8) //��ľ��ľ���ը
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
	//δ���õ���ǰ��  0:�����ʼ��Ϊ���ߣ�-1:���κ�Ч��
	//���õ��ߺ�   0:�ޣ�1:ը���䳤��2:ը�����࣬3:����+1

	std::vector<Vec2> V1; //�洢���Ͻ������ɵ��ߵĵ�
	std::vector<Vec2> V2; //�洢���½������ɵ��ߵĵ�

	//15*15�ĸ��ӷֳ�4�����ж�
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{

			if (prop_mp[i][j] == -1) //�޵������ɵ�����
			{
				continue;
			}

			if (i < 7 && j >= 7) //���ϣ����ڱ�������
			{
				V1.push_back(Vec2(i, j));
			}

			if (i >= 7 && j < 7) //���£����ڱ�������
			{
				V2.push_back(Vec2(i, j));
			}
			if ((i <= 6 && j <= 6) || (i >= 7 && j >= 7)) //���Ϻ����£��������һ������
			{
				int tmp = rand() % 10;
				if (tmp < 5)
				{
					prop_mp[i][j] = 0; //50%�޵���
				}
				else
				{
					prop_mp[i][j] = rand() % 3 + 1; //50%�е���
				}
			}
		}
	}
	prop_mp[5][5] = rand() % 3 + 1;	 //���Ͻ�100%�������ɵ�
	prop_mp[8][10] = rand() % 3 + 1; //���½�100%�������ɵ�
	bool isV1[100];					 //�Ƿ��Ѵ��ڵ���
	memset(isV1, 0, sizeof(isV1));
	int cnt1 = 5; //���½������5��
	while (cnt1 > 0)
	{
		int tmp = rand() % V1.size(); //�������
		if (!isV1[tmp])				  //��λ��û�е���
		{
			isV1[tmp] = 1; //��Ǹ�λ��
			cnt1--;
		}
	}
	for (int i = 0; i < V1.size(); ++i) //�������½Ƿ���
	{
		if (isV1[i]) //��λ�ñ����
		{
			prop_mp[V1[i].x][V1[i].y] = rand() % 3 + 1; //���õ���
		}
	}
	//���½������5��������ʵ��ͬ��
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
	return prop_mp; //���ص������ɵ�ͼ
}

//��ʼ�����������ʵ��������CREATEFUNC�е���
bool MapLayer::init()
{
	if (!Layer::init())
		return false; //����ʧ��

	/*����bgm*/
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("music/bgm2.mp3", true);

	/*��ʼ������*/
	srand(time(NULL));
	BombTag = std::vector<int>();
	ExplosionTag = std::vector<int>();

	/*��ȡ���ֽ��ͼ������������ؽ���*/
	TMXTiledMap *map = TMXTiledMap::create("map.tmx"); //����tmx��ͼ
	_map = map;										   //�洢tmx��ͼ��
	//   ss : ����ϵ�������ó�����������ű���
	ss = Director::sharedDirector()->getVisibleSize().height / _map->getContentSize().height;
	_map->setScale(ss);
	_map->setAnchorPoint(Vec2(0.5f, 0.5f));					//����ê��Ϊ��������
	_map->setPosition(vsable.width / 2, vsable.height / 2); //����λ��Ϊ�������ģ��Ա���������
	addChild(_map, 0);										//�򳡾�����tmx��ͼ
	collid = _map->getLayer("collid");						//��ȡtmx��ͼ�е���ײ��
	auto barrier = _map->getLayer("barrier");				//��ȡtmx��ͼ�е��ϰ����
	barrier0 = _map->getLayer("barrier0");					//��ȡtmx��ͼ�е��ϰ������ֲ�
	props = init_prop_mp();									//���ɵ��ߵ�ͼ
	//�ֽ��ϰ���㣬���ò����Ƭ���벢���볡����ʹ֮�����ﾫ��ͬ��
	for (int j = 0; j < 15; j++)
	{
		for (int i = 0; i < 15; i++)
		{
			if (barrier->getTileGIDAt(Vec2(i, j)) > 0)
			{											  //�����i��j��������Ƭ
				auto sp = barrier->getTileAt(Vec2(i, j)); //��ȡ��Ƭ����
				auto pos = sp->getPosition();			  //��ȡ��Ƭʵ��λ��
				barrier->removeTileAt(Vec2(i, j));		  //���ϰ����и���Ƭ�Ƴ������Ƴ��޷��������볡����
				sp->setScale(ss);
				sp->setAnchorPoint(Vec2(0, 0));
				sp->setPosition(pos * ss);
				auto p = pos * ss;
				addChild(sp, j * 15 + i); //�򳡾�����(i,j)λ�õ���Ƭ��������ZOrder
				barriersTag[tmxFromPos(p)] = TagOrder;
				sp->setTag(TagOrder++);
				/*sp->getTexture()->setAntiAliasTexParameters();//���������*/
				zorder[j] = j * 15 + i; //��ͬһ������ZOrderֵ����zorder����
			}
		}
	}
	//�������ﾫ�飬������tmx��ͼ�ж���������
	//����ߴ�Ϊ40*54
	auto play = _map->getObjectGroup("ren"); //��ȡ��������
	auto one = play->getObject("player1");	 //��ȡ����player1
	double x = one["x"].asDouble();
	double y = one["y"].asDouble();			  //��ȡplayer1��λ������
	auto play1 = Sprite::create("Role1.png"); //�������ﾫ��play1
	play1->setAnchorPoint(Vec2(0, 0));
	play1->setPosition(toInt(Vec2(x * ss, y * ss))); //����play1������player1��λ������
	play1->setScale(ss);
	this->addChild(play1, zorder[9] + 1); //�򳡾�����play1�������ó�ʼZOrderֵ
	//play1->getTexture()->setAntiAliasTexParameters();
	auto player1 = new player("jack"); //����play1����������
	playerAll[0] = player1;
	playAll[0] = play1;					   //��ӹ���
	auto two = play->getObject("player2"); //��ȡ����player2
	x = two["x"].asDouble();
	y = two["y"].asDouble();				  //��ȡplayer2��λ������
	auto play2 = Sprite::create("Role2.png"); //�������ﾫ��play2
	play2->setAnchorPoint(Vec2(0, 0));
	play2->setPosition(toInt(Vec2(x * ss, y * ss))); //����play2������player2��λ������
	play2->setScale(ss);
	this->addChild(play2, zorder[1] + 1); //�򳡾�����play2�������ó�ʼZOrderֵ
	//play2->getTexture()->setAntiAliasTexParameters();
	auto player2 = new player("rose"); //����play2����������
	playerAll[1] = player2;
	playAll[1] = play2; //��ӹ���

	/*���ü��̼�����ʹ��������ƶ�������ը��*/
	auto listenerKey = EventListenerKeyboard::create();							//��������������
	const float speed = 0.3f;													//�����ƶ��ٶȼ����������ٶ�
	listenerKey->onKeyPressed = ([=](EventKeyboard::KeyCode code, Event *event) //�����������¼���ֵΪ����������[=]Ϊ���ݸ����б�����
	{
		if (b1)
		{ //W��A��S��D������
			switch (code)
			{
			case EventKeyboard::KeyCode::KEY_W:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0)); //��ʼ�������ƶ�
				if (isCollid(Vec2(0, 50), play1->getPosition()))
				{ //�Ϸ�����ײ���
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(0, 50)); //�����ƶ�һ��50=��Ļ��С/������
				}
				auto pAmiFrame = Vector<SpriteFrame *>(); //��ʼ��֡��������
				b1 = 0;								   //��ֹW��A��S��D�������ٰ���

				//���붯��֡
				for (int j = 0; j < 6; j++)
				{
					auto p = SpriteFrame::create("Role1action.png", Rect(j * 39, 0, 38, 52));
					//39=ͼƬ���-1���ر�Ե��38=ͼƬ���-���������1���ر�Ե��52=ͼƬ�߶�-���������1���ر�Ե
					pAmiFrame.pushBack(p);
				}
				pAmiFrame.pushBack(SpriteFrame::create("Role1action.png", Rect(0, 0, 38, 52)));

				//��������
				auto animation = Animation::createWithSpriteFrames(pAmiFrame, speed / 7);
				auto animate = Animate::create(animation);
				auto spawn = Spawn::create(animate, moveBy, NULL); //ʹ����Ķ������ƶ�ͬʱ����

				play1->runAction(spawn); //�������ﶯ��
				break;
			}
			case EventKeyboard::KeyCode::KEY_A:

			{
				auto moveBy = MoveBy::create(speed, Vec2(0, 0));
				if (isCollid(Vec2(-50, 0), play1->getPosition()))
				{
				} //�󷽵���ײ���
				else
				{
					moveBy = MoveBy::create(speed, Vec2(-50, 0)); //�����ƶ�һ��
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
				{ //�·�����ײ���
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(0, -50)); //�����ƶ�һ��
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
				{ //�ҷ�����ײ���
				}
				else
				{
					moveBy = MoveBy::create(speed, Vec2(50, 0)); //�����ƶ�һ��
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
			auto p = tmxFromPos(play1->getPosition());						  //��ȡplay1��tmx��ͼ����
			this->reorderChild(play1, zorder[(int)p.y] + 1);				  //�ı�play1��ZOrder��ʵ���ϰ���������֮����ڵ���ϵ
			this->scheduleOnce(schedule_selector(MapLayer::b1ToOne), 0.25f); //��ʱ0.25��ʹ b1 = 1
		}
		if (b2)
		{ //�ϡ��¡����ұ����£�����ʵ��ͬ��
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
			this->scheduleOnce(schedule_selector(MapLayer::b2ToOne), 0.2f); //��ʱ
		}
		switch (code)
		{
		case EventKeyboard::KeyCode::KEY_SPACE: //�ո��������
		{
			if (player1->getBombNum() > 0) //���տ�ʹ��ը������������ը��
			{
				auto pos = play1->getPosition();
				int x = round(pos.x); //xȡ��
				int y = round(pos.y); //yȡ��
				int xs = x % 50;
				int ys = y % 50;										//����������ƫ����
				auto position = play1->getPosition() + Vec2(-xs, -ys); //ը������λ��
				if (BombExist(toInt(position)))
				{ //��ǰλ���Ѵ���ը��������
					break;
				}
				auto bomb = Sprite::create("Popo.png", Rect(0, 0, 38, 38)); //����ը����38=ͼƬ���-���������1���ر�Ե
				bomb->setPosition(toInt(position));						 //�����︽���ĸ��ӷ���ը��
				bomb->setScale(ss);
				bomb->setAnchorPoint(Vec2(0, 0));
				addChild(bomb, play1->getLocalZOrder() - 1); //�򳡾�������ը����������ZOrder
				bomb->setTag(TagOrder);					  //����ը������ı�ǩ���Ա�ӳ����е���
				BombTag.push_back(TagOrder++);				  //�洢��ǰը����ǩ��TagOrderֵ��һ
				player1->setBomb();						  //����ը��
				auto pAmi = Vector<SpriteFrame *>();		  //��ʼ��֡��������

				//���붯��֡
				for (int i = 0; i < 3; i++)
				{
					auto p = SpriteFrame::create("Popo.png", Rect(i * 39, 0, 38, 38));
					pAmi.pushBack(p);
				}

				//��������
				auto animation = Animation::createWithSpriteFrames(pAmi, 0.2f);
				auto animate = Animate::create(animation);
				auto seq = (ActionInterval *)Sequence::create(animate, NULL); //��������
				auto repeat = Repeat::create(seq, 3);						   //ʹ�����ظ�����3��

				auto fun = CallFunc::create([=]() { //�ص�������ʹը����ը
					this->bombExplosion(1);
				});
				auto rf = Sequence::create(repeat, fun, NULL); //�ϳ��ȷ���ը������ը�Ķ���
				bomb->runAction(rf);							//����ը������
			}
			break;
		}
		case EventKeyboard::KeyCode::KEY_ENTER: //�س��������£�����ʵ��ͬ��
		{
			if (player2->getBombNum() > 0)
			{
				auto pos = play2->getPosition(); //��Ļ������������� x , y ����������
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
	auto dispatcher = Director::getInstance()->getEventDispatcher(); //�����¼��ַ���
	//��Ӱ������������¼��ַ���
	dispatcher->addEventListenerWithSceneGraphPriority(listenerKey, this);
	this->scheduleUpdate(); //����֡��ʱ��
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

//ը����������������ը��������ӱ�ը����
//@param ����id
void MapLayer::bombExplosion(int id)
{
	if (BombTag.empty()) //������û��ը��
		return;
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("music/boom.mp3");
	player *p = playerAll[id - 1];
	auto bomb = (Sprite *)getChildByTag(BombTag.front()); //ȡ��BombTag�����ײ��ı�ǩ��
	Vec2 pos = bomb->getPosition();						  //����ը��λ��
	log("check:%f,%f", pos.x, pos.y);
	int z = bomb->getLocalZOrder();	   //����ը��ZOrder
	removeChildByTag(BombTag.front()); //�ӳ������Ƴ�ը������
	BombTag.erase(BombTag.begin());	   //�Ƴ�BombTag�����ը����ǩ

	//���ı�ըЧ��
	auto center = Sprite::create("Explosion.png", Rect(0, 4 * 39, 40, 40)); //�������ı�ը����
	center->setPosition(pos);
	center->setScale(ss);
	center->setAnchorPoint(Vec2(0, 0));
	addChild(center, z);				 //�򳡾��������ı�ը����
	center->setTag(TagOrder);			 //���ñ�ը����ı�ǩ���Ա�ӳ����е���
	ExplosionTag.push_back(TagOrder++);	 //�洢��ǰ��ը�����ǩ��TagOrderֵ��һ
	auto pAmi = Vector<SpriteFrame *>(); //��ʼ��֡��������

	//���붯��֡
	for (int i = 0; i < 8; i++)
	{
		auto p = SpriteFrame::create("Explosion.png", Rect(i % 4 * 39, 4 * 39, 40, 40));
		pAmi.pushBack(p);
	}
	pAmi.pushBack(SpriteFrame::create("Explosion.png", Rect(4 * 40, 4 * 40, 38, 38)));
	//����һ�ſհ׵���ͼ����Ϊ��ըЧ��������״̬

	//��������
	auto animation = Animation::createWithSpriteFrames(pAmi, 0.07f);
	auto animate = Animate::create(animation);
	center->runAction(animate); //�������ı�ը����

	//�Ϸ�
	for (int j = 1; j <= p->getBombLength(); j++)
	{												   //����ը���������ɱ�ը����
		auto check = isDestroy(pos + Vec2(0, 50 * j)); //�жϱ�ը�Ե�����ɵ�Ӱ��
		if (check == 2)
			break; //�߽���ӣ���ľ�赲��ը
		auto front = Sprite::create("Explosion.png", Rect(0, 0, 40, 40));
		front->setPosition(pos + Vec2(0, 50 * j)); //ʹ�Ϸ���ը����λ��ԭը���Ϸ�
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
		{ //��ľ�����ӱ���ը����
			auto pos_ = pos + Vec2(0, 50 * j);
			removeChildByTag(barriersTag[tmxFromPos(pos_)]);
			auto pso = tmxFromPos(pos_);
			collid->removeTileAt(pso);
			barrier0->removeTileAt(pso);
			revealProps(pos_.x, pos_.y, zorder[(int)pso.y]);
			break;
		}
	}

	//�·�������ʵ��ͬ��
	for (int j = 1; j <= p->getBombLength(); j++)
	{
		auto check = isDestroy(pos + Vec2(0, -50 * j));
		if (check == 2)
			break;
		auto down = Sprite::create("Explosion.png", Rect(0, 39, 40, 40));
		down->setPosition(pos + Vec2(0, -50 * j)); //ʹ�·���ը����λ��ԭը���·�
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

	//�󷽣�����ʵ��ͬ��
	for (int j = 1; j <= p->getBombLength(); j++)
	{
		auto check = isDestroy(pos + Vec2(-50 * j, 0));
		if (check == 2)
			break;
		auto left = Sprite::create("Explosion.png", Rect(0, 2 * 39, 40, 40));
		left->setPosition(pos + Vec2(-50 * j, 0)); //ʹ�󷽱�ը����λ��ԭը����
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

	//�ҷ�������ʵ��ͬ��
	for (int j = 1; j <= p->getBombLength(); j++)
	{
		auto check = isDestroy(pos + Vec2(50 * j, 0));
		if (check == 2)
			break;
		auto right = Sprite::create("Explosion.png", Rect(0, 3 * 39, 40, 40));
		right->setPosition(pos + Vec2(50 * j, 0)); //ʹ�ҷ���ը����λ��ԭը���ҷ�
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
	p->incBombNum(); //��ը��ϣ���ʹ��ը����������

	//��ʱ0.9s�����������ź��Ƴ���ը����
	scheduleOnce(schedule_selector(MapLayer::RemoveExplosion), 0.9f);
}

//��ײ��⺯��
//@param λ������ԭλ��
//@return falseδ��ײ��true������ײ
bool MapLayer::isCollid(Vec2 npos, Vec2 mpos)
{
	Vec2 pos = (npos + mpos);									//λ�ƺ�λ��
	Vec2 tile = tmxFromPos(pos);								//��ȡpos����Ӧ��tmx��ͼ����
	if (tile.x < 0 || tile.x > 14 || tile.y < 0 || tile.y > 14) //�߽���
		return true;
	int tileGid = collid->getTileGIDAt(tile); //��ȡtile����Ӧ����ƬGID
	if (tileGid > 0)
	{													 //tileλ�����ϰ���
		Value prop = _map->getPropertiesForGID(tileGid); //��ȡ��Ƭ������
		ValueMap propMap = prop.asValueMap();			 //������ת����Map
		bool colid = propMap["collid"].asBool();		 //��ȡ��Ƭcollid���Ե�ֵ
		if (colid)
		{ //collidΪ��
			//��ײ��Ч
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("music/wall.mp3");
			return true;
		}
	}
	return false;
}

//ը���ƻ���⺯��
//@param ���λ��
//@return 0:���ڵ���1:���ƻ���2:�߽�򲻿��ƻ�
int MapLayer::isDestroy(Vec2 pos)
{
	Vec2 tile = tmxFromPos(pos);								//��ȡpos����Ӧ��tmx��ͼ����
	if (tile.x < 0 || tile.x > 14 || tile.y < 0 || tile.y > 14) //�߽���
		return 2;
	int tileGid = barrier0->getTileGIDAt(tile); //��ȡtile����Ӧ����ƬGID
	switch (tileGid)
	{
	case 1: //1.��ľ
	case 2:
	case 3:
	case 4: //2.3.4.����
		return 2;
		break;
	case 6: //6.����
	case 7:
	case 8: //7.8.��ľ
		return 1;
		break;
	}
	return 0;
}

//��Ļ����->tmx��ͼ����
Vec2 MapLayer::tmxFromPos(Vec2 pos)
{
	//Vec2 pos_ = pos + Vec2(2.5, -2.5);//��Ļ������������� x , y ����������
	int x = round(pos.x / 50);		//�����������x����
	int y = 14 - round(pos.y / 50); //�����������y���꣬����ת����ϵ
	return Vec2(x, y);
}

//�Ƴ���ը����
//@param dt��ʵ��Ч����Ϊ��ʱ����ʹ�ã�����ȱ��
void MapLayer::RemoveExplosion(float dt)
{
	Vec2 poss[100];
	int ct = 0;
	for (int i = 0; i < ExplosionTag.size(); i++)
	{ //������ը��ǩ����
		poss[ct++] = getChildByTag(ExplosionTag[i])->getPosition();
		removeChildByTag(ExplosionTag[i]); //�Ƴ������б�ǩ����Ӧ�ľ���
	}
	ExplosionTag.clear(); //���ExplosionTag����
	killPlayer(poss, ct);
}

//֡��ʱ�����º���
void MapLayer::update(float dt)
{
	for (int i = 0; i < 2; ++i)
	{
		auto pos = toInt(playAll[i]->getPosition());
		int x = pos.x;
		int y = pos.y;
		if (x % 50 == 0 && y % 50 == 0)
		{ //���ﾲֹʱ���жϵ����Ƿ�ʰ��
			buffAndRemove(i + 1);
		}
	}
}

//��ɱ��Һ���
//@param ��ըλ��
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
		endString = "�ÿ�ϧѽ~��ƽ���أ�";
		//MessageBoxW(nullptr, L"��ϧ��ƽ��Ŷ��", L"Game Over", MB_OK);
		log("ƽ��");
	}
	else
	{
		if (playerAll[0]->getHealth() <= 0)
		{
			isWin = 1;
			endString = "��ϲ��Player2Ӯ��Ŷ��";
			//MessageBoxW(nullptr, L"Player2 Win!!!", L"Game Over", MB_OK);
			log("Player2 Win!!!");
		}
		else if (playerAll[1]->getHealth() <= 0)
		{
			isWin = 1;
			endString = "��ϲ��Player1Ӯ��Ŷ��";
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

//��ʾ����
//@param ��Ļx����,��Ļy���꣬ZOrder
void MapLayer::revealProps(int nx, int ny, int zorder)
{
	auto pos = tmxFromPos(Vec2(nx, ny)); //ת����ͼ����
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
	prop->setPosition(Vec2(nx, ny)); //���õ���
	prop->setScale(ss);
	prop->setAnchorPoint(Vec2(0, 0));
	addChild(prop, zorder - 15); //�򳡾���������ߣ�������ZOrder
	propsTag[toInt(Vec2(nx, ny))] = TagOrder;
	prop->setTag(TagOrder++);
	++propCount;
	auto pAmi = Vector<SpriteFrame *>(); //��ʼ��֡��������
	//���붯��֡
	for (int i = 0; i < 3; i++)
	{
		auto p = SpriteFrame::create(t, Rect(i * 39, 0, 38, 38));
		pAmi.pushBack(p);
	}

	//��������
	auto animation = Animation::createWithSpriteFrames(pAmi, 0.2f);
	auto animate = Animate::create(animation);
	auto seq = (ActionInterval *)Sequence::create(animate, NULL); //��������
	auto repeat = RepeatForever::create(seq);
	prop->runAction(repeat); //���붯��
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
