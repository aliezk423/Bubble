#pragma once
#include "cocos2d.h"

USING_NS_CC;

class StartLayer : public Layer
{
public:
	static Scene *createScene();
	virtual bool init();
	CREATE_FUNC(StartLayer);

private:
	Size vsable = {750, 750};
};
