#include "EndLayer.h"
#include "StartLayer.h"
#include "SimpleAudioEngine.h"

std::string str = "";

Scene *EndLayer::createScene()
{
	Scene *scene = Scene::create();
	EndLayer *endLayer = EndLayer::create();
	scene->addChild(endLayer);
	return scene;
}

bool EndLayer::init()
{
	if (!Layer::init())
		return false;
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("music/win.mp3");
	auto bg = Sprite::create("end.jpg", Rect(0, 0, 750, 750));
	bg->setAnchorPoint(Vec2(0.5, 0.5));
	bg->setPosition(vsable.width / 2, vsable.height / 2);
	addChild(bg, 1);
	auto label = LabelTTF::create(FontToUTF8(str.c_str()), "", 40);
	label->setColor(Color3B::BLACK);
	label->setPosition(260, 560);
	addChild(label, 2);
	auto item = MenuItemSprite::create(
		Sprite::create("home.png"),
		Sprite::create("home1.png"),
		[&](Ref *ref) {
			Director::getInstance()->replaceScene(StartLayer::createScene());
		});
	item->setPosition(200, 130);
	auto menu = Menu::create(item, NULL);
	menu->setPosition(Point::ZERO);
	addChild(menu, 3);
	return true;
}

char *EndLayer::FontToUTF8(const char *font)
{
	int len = MultiByteToWideChar(CP_ACP, 0, font, -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, font, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char *str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr)
		delete[] wstr;
	return str;
}
