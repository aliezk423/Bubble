#pragma once
#include <string>
#include <vector>

class player
{
public:
	player(std::string name);
	int x, y;
	std::string getName();
	void setBomb();
	int incBombLength();
	int incBombNum();
	int incHealth();
	int decHealth();
	int getBombLength();
	int getBombNum();
	int getHealth();

private:
	std::string name;
	int bombLength;
	int bombNum;
	int health;
};
