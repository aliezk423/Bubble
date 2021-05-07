#include "player.h"
#include <vector>
#include <string>;

player::player(std::string name)
{
	this->name = name;
	bombLength = 1;
	bombNum = 1;
	health = 1;
}

std::string player::getName()
{
	return name;
}

void player::setBomb()
{
	if (bombNum > 0)
	{
		--bombNum;
	}
}
int player::incBombLength()
{
	return ++bombLength;
}
int player::incBombNum()
{
	return ++bombNum;
}
int player::incHealth()
{
	return ++health;
}
int player::decHealth()
{
	return --health;
}
int player::getBombLength()
{
	return bombLength;
}
int player::getBombNum()
{
	return bombNum;
}
int player::getHealth()
{
	return health;
}