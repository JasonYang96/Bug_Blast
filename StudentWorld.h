#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include "Actor.h"
#include <vector>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
	StudentWorld()
	{
		m_actors.clear();
		m_sprayerCount = 0;
		m_maxSprayers = 2;
		m_numZumi = 0;
		m_finishedLevel = false;
	}

	~StudentWorld()
	{
		delete m_player;
		delete m_level;
		for (unsigned int k = 0; k < m_actors.size(); k++)
			delete m_actors[k];
		m_actors.clear();
	}

	//accessors
	std::vector<Actor*> getActors() const { return m_actors; }
	Player* getPlayer() const { return m_player; }
	Level* getLev() const { return m_level; }
	int getSprayerCount() const { return m_sprayerCount; }


	//mutators
	virtual int init();
	virtual int move();
	virtual void cleanUp()
    {
		delete m_player;
		delete m_level;
		for (unsigned int k = 0; k < m_actors.size(); k++)
			delete m_actors[k];
		m_actors.clear();
    }
	void FinishedLevel() { m_finishedLevel = true; }
	void GoodieDrop(int num, int x, int y);

	//helper functions

	//General
	int  whatActorIsThis(Actor* ptr);
	Actor* WhatActorHere(int x, int y);
	bool isBrick(int x, int y);
	void deadZumi() { m_numZumi--; }
	void deadActor(int x, int y, int id);

	//BugSpray or BugSprayer
	void resetBugSprayers() { m_maxSprayers = 2; }
	void createBugSprayer();
	void maxBugSprayers(){ m_maxSprayers = getLev()->getOptionValue(optionMaxBoostedSprayers); }
	void createBugSpray(int x, int y);
	void deadBugSprayer(){ m_sprayerCount--; }
	void checkSprayer(int x, int y);

	//Complex Zumi
	int path(int sx, int sy, int ex, int ey)
	{
		return pathExists(maze, sx, sy, ex, ey);
	}

	//Goodies
	int  getExtraProb() { return ProbOfExtraLifeGoodie; }
	int  getBonus() { return LevelBonus; }
	int  getThroughWallsProb() { return ProbOfWalkThruGoodie; }
	int  getBoosterProb() { return ProbOfMoreSprayersGoodie; }

private:
	Level*	m_level;
	Player*	m_player;
	std::vector<Actor*>	m_actors;
	int TicksPerSimpleZumiMove;
	int TicksPerComplexZumiMove;
	int ProbOfGoodieOverall;
	int ProbOfExtraLifeGoodie;
	int ProbOfWalkThruGoodie;
	int ProbOfMoreSprayersGoodie;
	int GoodieLifetimeInTicks;
	int WalkThruLifetimeTicks;
	int BoostedSprayerLifetimeTicks;
	int MaxBoostedSprayers;
	int LevelBonus;
	int m_sprayerCount;
	int m_maxSprayers;
	int m_numZumi;
	bool m_finishedLevel;

	//private helper functions
	void setDisplayText();

	class CoordDirection
	{
	public:
		CoordDirection(int x, int y, int dir) : m_col(x), m_row(y), m_direction(dir) {}
		int x() const { return m_col; }
		int y() const { return m_row; }
		int dir() const { return m_direction; }
	private:
		int m_row;
		int m_col;
		int m_direction;
	};

	bool maze[VIEW_HEIGHT][VIEW_WIDTH];
	int pathExists(bool maze[][15], int sr, int sc, int er, int ec);

};

#endif // STUDENTWORLD_H_
