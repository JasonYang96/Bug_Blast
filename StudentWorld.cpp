#include "StudentWorld.h"
#include "Level.h"
#include "Actor.h"
#include "GameConstants.h"
#include <time.h>
#include <sstream>
#include <queue>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld()
{
	return new StudentWorld();
}

int StudentWorld::init()
{
	//initialize data structures used to keep track of the game's world
	m_actors.clear();
	m_sprayerCount = 0;
	m_maxSprayers = 2;
	m_numZumi = 0;
	m_finishedLevel = false;
	srand(time(NULL));

	//load current maze details
	string curLevel = "";
	switch (getLevel())
	{
	case 0:
		curLevel = "level00.dat";
		break;
	case 1:
		curLevel = "level01.dat";
		break;
	case 2:
		curLevel = "level02.dat";
		break;
	default:
		curLevel = "Player won";
		break;
	}

	m_level = new Level();
	Level::LoadResult result = m_level->loadLevel(curLevel);

	switch (result)
	{
	case Level::load_fail_file_not_found:
		if (getLevel() == 0)
			return GWSTATUS_NO_FIRST_LEVEL;
		else
			return GWSTATUS_PLAYER_WON;
		break;
	case Level::load_fail_bad_format:
		return GWSTATUS_LEVEL_ERROR;
		break;
	default:
		break;
	}

	//load level options
	ProbOfGoodieOverall = m_level->getOptionValue(optionProbOfGoodieOverall);
	ProbOfExtraLifeGoodie = m_level->getOptionValue(optionProbOfExtraLifeGoodie);
	ProbOfWalkThruGoodie = m_level->getOptionValue(optionProbOfWalkThruGoodie);
	ProbOfMoreSprayersGoodie = m_level->getOptionValue(optionProbOfMoreSprayersGoodie);
	TicksPerSimpleZumiMove = m_level->getOptionValue(optionTicksPerSimpleZumiMove);
	TicksPerComplexZumiMove = m_level->getOptionValue(optionTicksPerComplexZumiMove);
	GoodieLifetimeInTicks = m_level->getOptionValue(optionGoodieLifetimeInTicks);
	LevelBonus = m_level->getOptionValue(optionLevelBonus);
	WalkThruLifetimeTicks = m_level->getOptionValue(optionWalkThruLifetimeTicks);
	BoostedSprayerLifetimeTicks = m_level->getOptionValue(optionBoostedSprayerLifetimeTicks);
	MaxBoostedSprayers = m_level->getOptionValue(optionMaxBoostedSprayers);

	//allocate objects
	for (unsigned int i = 0; i < VIEW_WIDTH; i++)
	{
		for (unsigned int j = 0; j < VIEW_HEIGHT; j++)
		{
			if (m_level->getContentsOf(i, j) == Level::player)
				m_player = new Player(i, j, this);
			if (m_level->getContentsOf(i, j) == Level::destroyable_brick)
			{
				DestroyableBrick* db = new DestroyableBrick(i, j, this);
				m_actors.push_back(db);
			}
			if (m_level->getContentsOf(i, j) == Level::perma_brick)
			{
				PermaBrick* pb = new PermaBrick(i, j, this);
				m_actors.push_back(pb);
			}
			if (m_level->getContentsOf(i, j) == Level::exit)
			{
				Exit* exit = new Exit(i, j, this);
				m_actors.push_back(exit);
			}
			if (m_level->getContentsOf(i, j) == Level::simple_zumi)
			{
				SimpleZumi* sz = new SimpleZumi(i, j, this);
				m_actors.push_back(sz);
				m_numZumi++;
			}
			if (m_level->getContentsOf(i, j) == Level::complex_zumi)
			{
				ComplexZumi* cz = new ComplexZumi(i, j, this);
				m_actors.push_back(cz);
				m_numZumi++;
			}
		}
	}

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	//Update the Game Status Line
	setDisplayText();

	//all Actors doSomething()
	for (unsigned int k = 0; k < m_actors.size(); k++)
	{
		if (m_actors[k]->isAlive())
			m_actors[k]->doSomething();
		if (!(m_actors[k]->isAlive()))
		{
			delete m_actors[k];
			m_actors.erase(m_actors.begin() + k);
		}
	}

	if (getPlayer()->isAlive())
	{
		m_player->doSomething();
		if (!(getPlayer()->isAlive()))
			return GWSTATUS_PLAYER_DIED;
	}
	else
		return GWSTATUS_PLAYER_DIED;

	LevelBonus--;

	//if all Zumi are dead, expose Exit
	if (m_numZumi == 0)
	{
		for (unsigned int k = 0; k < m_actors.size(); k++)
		{
			if (whatActorIsThis(m_actors[k]) == IID_EXIT)
				dynamic_cast<Exit*>(m_actors[k])->revealExit();
		}
	}

	if (m_finishedLevel)
		return GWSTATUS_FINISHED_LEVEL;

	//Player hasn't completed current level and hasn't died, so
	//continue playing the current level
	return GWSTATUS_CONTINUE_GAME;
}

bool StudentWorld::isBrick(int x, int y)
{
	Actor* object = WhatActorHere(x, y);
	PermaBrick* pb = dynamic_cast<PermaBrick*>(object);
	DestroyableBrick* db = dynamic_cast<DestroyableBrick*>(object);
	//if permabrick
	if (pb)
		return true;
	//if destroyable brick and player can walk through walls
	else if (db && getPlayer()->WalkThroughWallsLife() > 0)
		return false;
	//if destroyable brick and player cannot walk through walls
	else if (db)
		return true;

	return false;
}

void StudentWorld::createBugSprayer()
{
	//making sure not on destroyable brick or bug sprayer
	bool onItem = false;
	for (unsigned int k = 0; k < m_actors.size(); k++)
	{
		if ((whatActorIsThis(m_actors[k]) == IID_DESTROYABLE_BRICK ||
			whatActorIsThis(m_actors[k]) == IID_BUGSPRAYER) &&
			m_actors[k]->getX() == getPlayer()->getX() &&
			m_actors[k]->getY() == getPlayer()->getY())
			onItem = true;
	}
	if (onItem)
		return;

	//making sure Sprayer Count does not exceed Max Sprayers
	if (m_sprayerCount >= m_maxSprayers)
		return;

	//creating Bugsprayer
	BugSprayer* bs = new BugSprayer(m_player->getX(), m_player->getY(), this);
	m_actors.push_back(bs);
	m_sprayerCount++;
}

void StudentWorld::createBugSpray(int x, int y)
{
	BugSpray* bs = new BugSpray(x, y, this);
	m_actors.push_back(bs);

	//create bugspray in the right direction
	for (int k = 1; k <= 2; k++)
	{
		PermaBrick* pb = dynamic_cast<PermaBrick*>(WhatActorHere(x + k, y));
		if (!pb)
		{
			BugSpray* bs = new BugSpray(x + k, y, this);
			m_actors.push_back(bs);
		}
		Brick* brick = dynamic_cast<Brick*>(WhatActorHere(x + k, y));
		if (brick)
			break;
	}

	//create bugspray in the left direction
	for (int k = 1; k <= 2; k++)
	{
		PermaBrick* pb = dynamic_cast<PermaBrick*>(WhatActorHere(x - k, y));
		if (!pb)
		{
			BugSpray* bs = new BugSpray(x - k, y, this);
			m_actors.push_back(bs);
		}
		Brick* brick = dynamic_cast<Brick*>(WhatActorHere(x - k, y));
		if (brick)
			break;
	}

	//create bugspray in the up direction
	for (int k = 1; k <= 2; k++)
	{
		PermaBrick* pb = dynamic_cast<PermaBrick*>(WhatActorHere(x, y + k));
		if (!pb)
		{
			BugSpray* bs = new BugSpray(x, y + k, this);
			m_actors.push_back(bs);
		}
		Brick* brick = dynamic_cast<Brick*>(WhatActorHere(x, y + k));
		if (brick)
			break;
	}

	//create bugspray in the down direction
	for (int k = 1; k <= 2; k++)
	{
		PermaBrick* pb = dynamic_cast<PermaBrick*>(WhatActorHere(x, y - k));
		if (!pb)
		{
			BugSpray* bs = new BugSpray(x, y - k, this);
			m_actors.push_back(bs);
		}
		Brick* brick = dynamic_cast<Brick*>(WhatActorHere(x, y - k));
		if (brick)
			break;
	}
}

void StudentWorld::deadActor(int x, int y, int id)
{
	//setDead() actor
	for (unsigned int k = 0; k < m_actors.size(); k++)
	{
		if (whatActorIsThis(m_actors[k]) == id &&
			m_actors[k]->getX() == x &&
			m_actors[k]->getY() == y)
			m_actors[k]->setDead();
	}
}

void StudentWorld::checkSprayer(int x, int y)
{
	for (unsigned int k = 0; k < m_actors.size(); k++)
	{
		//if spray is on BugSprayer, set BugSprayer dead
		if (whatActorIsThis(m_actors[k]) == IID_BUGSPRAYER)
		if (m_actors[k]->getX() == x)
		if (m_actors[k]->getY() == y)
		{
			m_actors[k]->setDead();
		}
	}
}

int StudentWorld::whatActorIsThis(Actor* ptr)
{
	if (ptr != NULL)
	{
		PermaBrick* pb = dynamic_cast<PermaBrick*>(ptr);
		if (pb != NULL)
			return IID_PERMA_BRICK;
		DestroyableBrick* db = dynamic_cast<DestroyableBrick*>(ptr);
		if (db != NULL)
			return IID_DESTROYABLE_BRICK;
		Exit* ex = dynamic_cast<Exit*>(ptr);
		if (ex != NULL)
			return IID_EXIT;
		BugSprayer* sprayer = dynamic_cast<BugSprayer*>(ptr);
		if (sprayer != NULL)
			return IID_BUGSPRAYER;
		BugSpray* spray = dynamic_cast<BugSpray*>(ptr);
		if (spray != NULL)
			return IID_BUGSPRAY;
		ExtraLifeGoodie* elg = dynamic_cast<ExtraLifeGoodie*>(ptr);
		if (elg != NULL)
			return IID_EXTRA_LIFE_GOODIE;
		WalkThroughWallsGoodie* wtwg = dynamic_cast<WalkThroughWallsGoodie*>(ptr);
		if (wtwg != NULL)
			return IID_WALK_THRU_GOODIE;
		BoostSprayerGoodie* bs = dynamic_cast<BoostSprayerGoodie*>(ptr);
		if (bs != NULL)
			return IID_INCREASE_SIMULTANEOUS_SPRAYER_GOODIE;
		SimpleZumi* sz = dynamic_cast<SimpleZumi*>(ptr);
		if (sz != NULL)
			return IID_SIMPLE_ZUMI;
		ComplexZumi* cz = dynamic_cast<ComplexZumi*>(ptr);
		if (cz != NULL)
			return IID_COMPLEX_ZUMI;
	}
	return -1;
}

Actor* StudentWorld::WhatActorHere(int x, int y)
{
	for (unsigned int k = 0; k < m_actors.size(); k++)
	{
		if (m_actors[k]->getX() == x &&
			m_actors[k]->getY() == y)
			return m_actors[k];
	}
	return nullptr;
}

void StudentWorld::GoodieDrop(int num, int x, int y)
{
	if (num == 0)
	{
		Actor* life = new ExtraLifeGoodie(x, y, this);
		m_actors.push_back(life);
	}
	else if (num == 1)
	{
		Actor* ThruWalls = new WalkThroughWallsGoodie(x, y, this);
		m_actors.push_back(ThruWalls);
	}
	else
	{
		Actor* Boost = new BoostSprayerGoodie(x, y, this);
		m_actors.push_back(Boost);
	}
}

void StudentWorld::setDisplayText()
{
	int syore = getScore();
	int level = getLevel();
	int bonus = getBonus();
	int livesLeft = getLives();

	ostringstream s;
	s.fill('0');
	s << "syore: " << setw(7) << syore;
	s << "  Level: " << level;
	s << "  Lives: " << livesLeft;
	s << "  Bonus: " << bonus;
	string text = s.str();
	setGameStatText(text);
}

int StudentWorld::pathExists(bool maze[][15], int sx, int sy, int ex, int ey)
{
	for (int k = 0; k < VIEW_HEIGHT; k++)
	{
		for (int j = 0; j < VIEW_WIDTH; j++)
		{
			maze[k][j] = false;
		}
	}

	//declare a queue of CoordDirections
	queue<CoordDirection> m;

	//push starting CoordDirections and update maze[]

	//pushing first up direction
	Actor* up = WhatActorHere(sx, sy + 1);
	Brick* upbrick = dynamic_cast<Brick*>(up);
	if (!upbrick)
	{
		m.push(CoordDirection(sx, sy + 1, KEY_PRESS_UP));
		maze[sx][sy + 1] = true;
	}
	//pushing first right direction
	Actor* right = WhatActorHere(sx + 1, sy);
	Brick* rightbrick = dynamic_cast<Brick*>(right);
	if (!rightbrick)
	{
		m.push(CoordDirection(sx + 1, sy, KEY_PRESS_RIGHT));
		maze[sx + 1][sy] = true;
	}
	//pushing first down direction
	Actor* down = WhatActorHere(sx, sy - 1);
	Brick* downbrick = dynamic_cast<Brick*>(down);
	if (!downbrick)
	{
		m.push(CoordDirection(sx, sy - 1, KEY_PRESS_DOWN));
		maze[sx][sy - 1] = true;
	}
	//pushing first left direction
	Actor* left = WhatActorHere(sx - 1, sy);
	Brick* leftbrick = dynamic_cast<Brick*>(left);
	if (!leftbrick)
	{
		m.push(CoordDirection(sx - 1, sy, KEY_PRESS_LEFT));
		maze[sx - 1][sy] = true;
	}

	while (m.empty() == false)
	{
		//getting front CoordDirection and popping it
		CoordDirection temp = m.front();
		int direction = m.front().dir();
		m.pop();

		//checking if current CoordDirection is end CoordDirection
		if (temp.x() == ex && temp.y() == ey)
			return direction;

		//checking each direction for walkspace
		//checking NORTH direction
		Actor* up = WhatActorHere(temp.x(), temp.y() + 1);
		Brick* upbrick = dynamic_cast<Brick*>(up);
		if (!(upbrick || maze[temp.x()][temp.y() + 1]))
		{
			m.push(CoordDirection(temp.x(), temp.y() + 1, direction));
			maze[temp.x()][temp.y() + 1] = true;
		}
		//checking EAST direction
		Actor* right = WhatActorHere(temp.x() + 1, temp.y());
		Brick* rightbrick = dynamic_cast<Brick*>(right);
		if (!(rightbrick || maze[temp.x() + 1][temp.y()]))
		{
			m.push(CoordDirection(temp.x() + 1, temp.y(), direction));
			maze[temp.x() + 1][temp.y()] = true;
		}
		//checking SOUTH direction
		Actor* down = WhatActorHere(temp.x(), temp.y() - 1);
		Brick* downbrick = dynamic_cast<Brick*>(down);
		if (!(downbrick || maze[temp.x()][temp.y() - 1]))
		{
			m.push(CoordDirection(temp.x(), temp.y() -1, direction));
			maze[temp.x()][temp.y() - 1] = true;
		}
		//checking WEST direction
		Actor* left = WhatActorHere(temp.x() - 1, temp.y());
		Brick* leftbrick = dynamic_cast<Brick*>(left);
		if (!(leftbrick || maze[temp.x() - 1][temp.y()]))
		{
			m.push(CoordDirection(temp.x() - 1, temp.y(), direction));
			maze[temp.x() - 1][temp.y()] = true;
		}
	}
	//no solution
	return -1;
}