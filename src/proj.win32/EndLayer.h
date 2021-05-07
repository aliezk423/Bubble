#pragma once
#include "cocos2d.h"
USING_NS_CC;

extern std::string str;

class EndLayer : public Layer
{
public:
	static Scene *createScene();
	virtual bool init();
	char *FontToUTF8(const char *font);
	CREATE_FUNC(EndLayer);

private:
	Size vsable = {750, 750};
};
