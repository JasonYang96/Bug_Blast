#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
//Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
#include "GameConstants.h"

class StudentWorld;

class Actor : public GraphObject
{
public:
	Actor(int imageID, int startX, int startY, StudentWorld* world)
		:GraphObject(imageID, startX, startY)
	{
		m_world = world;
		m_alive = true;
		setVisible(true);
	}
	virtual ~Actor() {}

	//accessors
	
	//return StudentWorld pointer it lvies in
	StudentWorld* getWorld() { return m_world; }

	//return whether actor is alive
	bool isAlive() { return m_alive; }

	//mutators

	//pure virtual doSomething for each Actor
	virtual void doSomething() = 0;

	//virtual function to setDead() Actors
	virtual void setDead()
	{
		setVisible(false);
		m_alive = false;
	}

private:
	StudentWorld* m_world;
	bool		  m_alive;
};

class Brick : public Actor
{
public:
	Brick(int imageID, int startX, int startY, StudentWorld* world)
		:Actor(imageID, startX, startY, world) {}
	virtual ~Brick() {}

	void doSomething() {}

private:

};

class DestroyableBrick : public Brick
{
public:
	DestroyableBrick(int startX, int startY, StudentWorld* world)
		:Brick(IID_DESTROYABLE_BRICK, startX, startY, world) {}
	virtual ~DestroyableBrick() {}

private:

};

class PermaBrick : public Brick
{
public:
	PermaBrick(int startX, int startY, StudentWorld* world)
		:Brick(IID_PERMA_BRICK, startX, startY, world) {}
	virtual ~PermaBrick() {}

private:

};

class Player : public Actor
{
public:
	//Constructor and Destructor
	Player(int startX, int startY, StudentWorld* world)
		: Actor(IID_PLAYER, startX, startY, world)
	{
		int m_WalkThroughWallsTicks = 0;
		int m_BoostSprayerTicks = 0;
	}
	virtual ~Player() {}

	//accessors
	int WalkThroughWallsLife() { return m_WalkThroughWallsTicks; }
	int BoostSprayerLife() { return m_BoostSprayerTicks; }

	//mutators
	virtual void doSomething();
	virtual void setDead();
	void PickUpWalkThroughWallsGoodie();
	void PickUpBoosterSprayerGoodie();

private:
	int  m_WalkThroughWallsTicks;
	int  m_BoostSprayerTicks;
};

class TimedLifeActor : public Actor
{
public:
	TimedLifeActor(int IID, int startX, int startY, StudentWorld* world)
		: Actor(IID, startX, startY, world) {}
	virtual ~TimedLifeActor() {}

	//accessors
	int getTicks() { return m_ticks; }

	//mutators
	void setTicks(int ticks) { m_ticks = ticks; }

private:
	int m_ticks;
};

class BugSprayer : public TimedLifeActor
{
public:
	//constructor and destructor
	BugSprayer(int startX, int startY, StudentWorld* world)
		: TimedLifeActor(IID_BUGSPRAYER, startX, startY, world)
	{
		setTicks(40);
	}
	virtual ~BugSprayer() {};

	//mutators
	virtual void doSomething();
	virtual void setDead();
	void gotSpray() { setTicks(0); }

private:
};

class BugSpray : public TimedLifeActor
{
public:
	BugSpray(int startX, int startY, StudentWorld* world)
		: TimedLifeActor(IID_BUGSPRAY, startX, startY, world)
	{
		setTicks(3);
	}
	virtual ~BugSpray() {};

	//mutators
	virtual void doSomething();

private:

};

class Exit : public Actor
{
public:
	//Constructor and Destructor
	Exit(int startX, int startY, StudentWorld* world)
		: Actor(IID_EXIT, startX, startY, world)
	{
		setVisible(false);
		m_revealed = false;
	}
	virtual ~Exit() {}

	//mutators
	virtual void doSomething();
	void revealExit();

private:
	bool m_revealed;

};

class Zumi : public Actor
{
public:
	//Constructor and Destructor
	Zumi(int IID, int startX, int startY, StudentWorld* world);
	virtual ~Zumi(){}

	//accessors
	int returnTicks() const { return currentTick; }
	int returnDirection() const { return currentDirection; }

	//mutators
	virtual void doSomething();
	virtual void setDead();
	void move();
	void DropGoodie(int x, int y);
	void reset() { currentTick = TicksPerZumiMove; }
	void resetDirection() { currentDirection = rand() % 4 + 1000; }
	void changeDirection(int direction) { currentDirection = direction; }

private:
	int currentDirection;
	int TicksPerZumiMove;
	int currentTick;
	
};

class SimpleZumi : public Zumi
{
public:
	SimpleZumi(int startX, int startY, StudentWorld* world)
		: Zumi(IID_SIMPLE_ZUMI, startX, startY, world)
	{
	}
	virtual ~SimpleZumi() {}
	virtual void doSomething();

private:
};

class ComplexZumi : public Zumi
{
public:
	ComplexZumi(int startX, int startY, StudentWorld* world)
		: Zumi(IID_COMPLEX_ZUMI, startX, startY, world)
	{
	}
	virtual ~ComplexZumi() {}
	virtual void doSomething();

private:
};

class Goodie : public TimedLifeActor
{
public:
	Goodie(int IID, int startX, int startY, StudentWorld* world);
	virtual ~Goodie() {}

	//accessors

	//mutators
	virtual void doSomething()
	{
		if (!isAlive())
			return;

		setTicks(getTicks() - 1);

		if (getTicks() == 0)
			setDead();
	}

private:
};

class ExtraLifeGoodie : public Goodie
{
public:
	ExtraLifeGoodie(int startX, int startY, StudentWorld* world)
		: Goodie(IID_EXTRA_LIFE_GOODIE, startX, startY, world)
	{
	}
	virtual ~ExtraLifeGoodie() {}

	virtual void doSomething();

private:

};

class WalkThroughWallsGoodie : public Goodie
{
public:
	WalkThroughWallsGoodie(int startX, int startY, StudentWorld* world)
		: Goodie(IID_WALK_THRU_GOODIE, startX, startY, world)
	{
	}
	virtual ~WalkThroughWallsGoodie() {}

	virtual void doSomething();

private:


};

class BoostSprayerGoodie : public Goodie
{
public:
	BoostSprayerGoodie(int startX, int startY, StudentWorld* world)
		: Goodie(IID_INCREASE_SIMULTANEOUS_SPRAYER_GOODIE, startX, startY, world)
	{
	}
	virtual ~BoostSprayerGoodie() {}

	virtual void doSomething();

private:


};
#endif // ACTOR_H_
