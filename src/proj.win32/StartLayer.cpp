#include "StartLayer.h"
#include "MapLayer.h"
#include "EndLayer.h"
#include "SimpleAudioEngine.h"

Scene *StartLayer::createScene()
{
	Scene *scene = Scene::create();
	StartLayer *startLayer = StartLayer::create();
	scene->addChild(startLayer);
	return scene;
}

bool StartLayer::init()
{
	if (!Layer::init())
		return false;
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("music/bgm.mp3", true);
	auto bg = Sprite::create("start.jpg", Rect(0, 0, 750, 750));
	bg->setAnchorPoint(Vec2(0.5, 0.5));
	bg->setPosition(vsable.width / 2, vsable.height / 2);
	addChild(bg);
	auto item = MenuItemSprite::create(
		Sprite::create("button.png"),
		Sprite::create("button1.png"),
		[&](Ref *ref) {
			Director::getInstance()->replaceScene(MapLayer::createScene());
		});
	item->setPosition(375, 150);
	auto menu = Menu::create(item, NULL);
	menu->setPosition(Point::ZERO);
	addChild(menu);
	return true;
}
