#include "Actor.h"
#include "StudentWorld.h"
#include <cmath>

//PLAYER

void Player::doSomething()
{
	//check if dead
	if (!isAlive())
		return;
	
	//check if occupying forbidden objects
	for (unsigned int k = 0; k < getWorld()->getActors().size(); k++)
	{
		Actor* ptr = getWorld()->getActors()[k];

		//check if occupying Destroyable Brick
		if (m_WalkThroughWallsTicks <= 0 &&
			getWorld()->whatActorIsThis(ptr) == IID_DESTROYABLE_BRICK &&
			getWorld()->getActors()[k]->getX() == getX() &&
			getWorld()->getActors()[k]->getY() == getY())
			setDead();

		//check if occupying Zumis
		if ((getWorld()->whatActorIsThis(ptr) == IID_COMPLEX_ZUMI ||
			getWorld()->whatActorIsThis(ptr) == IID_SIMPLE_ZUMI) &&
			getWorld()->getActors()[k]->getX() == getX() &&
			getWorld()->getActors()[k]->getY() == getY())
			setDead();
	}

	//Decrementing Power Up States
	if (m_WalkThroughWallsTicks > 0)
		m_WalkThroughWallsTicks--;

	if (m_BoostSprayerTicks > 0)
		m_BoostSprayerTicks--;

	if (m_BoostSprayerTicks == 0)
		getWorld()->resetBugSprayers();

	//check for Player input
	int ch;
	if (getWorld()->getKey(ch))
	{
		switch (ch)
		{
		case KEY_PRESS_LEFT:
			if (!(getWorld()->isBrick(getX() - 1, getY())))
				moveTo(getX() - 1, getY());
			break;
		case KEY_PRESS_RIGHT:
			if (!(getWorld()->isBrick(getX() + 1, getY())))
				moveTo(getX() + 1, getY());
			break;
		case KEY_PRESS_UP:
			if (!(getWorld()->isBrick(getX(), getY() + 1)))
				moveTo(getX(), getY() + 1);
			break;
		case KEY_PRESS_DOWN:
			if (!(getWorld()->isBrick(getX(), getY() - 1)))
				moveTo(getX(), getY() - 1);
			break;
		case KEY_PRESS_SPACE:
			getWorld()->createBugSprayer();
			break;
		}
	}
}

void Player::setDead()
{
	Actor::setDead();
	getWorld()->playSound(SOUND_PLAYER_DIE);
	getWorld()->decLives();
}

void Player::PickUpWalkThroughWallsGoodie()
{
	//allow Player to walk through walks for WalkThruLifetimeTicks ticks
	m_WalkThroughWallsTicks = getWorld()->getLev()->getOptionValue(optionWalkThruLifetimeTicks);
}

void Player::PickUpBoosterSprayerGoodie()
{
	//allow player to drop maxBoosterSprayers for boostedSprayerLifetimeTicks
	m_BoostSprayerTicks = getWorld()->getLev()->getOptionValue(optionBoostedSprayerLifetimeTicks);
	getWorld()->maxBugSprayers();
}

//BUGSPRAYER

void BugSprayer::doSomething()
{
	if (!isAlive())
		return;

	setTicks(getTicks() - 1);

	//if dead, explode
	if (getTicks() == 0)
	{
		setDead();
	}
}

void BugSprayer::setDead()
{
	Actor::setDead();
	getWorld()->playSound(SOUND_SPRAY);
	getWorld()->deadBugSprayer();
	getWorld()->createBugSpray(getX(), getY());
}

//BUGSPRAY

void BugSpray::doSomething()
{
	if (!(isAlive()))
		return;

	setTicks(getTicks() - 1);

	if (getTicks() == 0)
		setDead();
	else
	{
		//Destroy Destroyable Brick
		if (getWorld()->getLev()->getContentsOf(getX(), getY()) == Level::destroyable_brick)
		{
			getWorld()->deadActor(getX(), getY(), IID_DESTROYABLE_BRICK);
		}

		//Destroy Player
		if (getWorld()->getPlayer()->getX() == getX() &&
			getWorld()->getPlayer()->getY() == getY())
		{
			getWorld()->getPlayer()->setDead();
		}

		//Destroy Zumi's
		Actor* actor = getWorld()->WhatActorHere(getX(), getY());
		SimpleZumi* sz = dynamic_cast<SimpleZumi*>(actor);
		ComplexZumi* cz = dynamic_cast<ComplexZumi*>(actor);
		if (sz)
			getWorld()->deadActor(getX(), getY(), IID_SIMPLE_ZUMI);
		else if (cz)
			getWorld()->deadActor(getX(), getY(), IID_COMPLEX_ZUMI);

		//Destory other Sprayers
		if (getWorld()->getSprayerCount() >= 1)
		{
			getWorld()->checkSprayer(getX(), getY());
		}
	}
}

//EXIT

void Exit::doSomething()
{
	//if active and player is on exit
	if (m_revealed && 
		getWorld()->getPlayer()->getX() == getX() &&
		getWorld()->getPlayer()->getY() == getY())
	{
		getWorld()->playSound(SOUND_FINISHED_LEVEL);
		getWorld()->FinishedLevel();
		getWorld()->increaseScore(getWorld()->getBonus());
	}
}

void Exit::revealExit()
{
	//set Exit active
	if (!m_revealed)
	{
		setVisible(true);
		m_revealed = true;
		getWorld()->playSound(SOUND_REVEAL_EXIT);
	}
}

//ZUMI

Zumi::Zumi(int IID, int startX, int startY, StudentWorld* world)
: Actor(IID, startX, startY, world)
{
	currentDirection = rand() % 4 + 1000;
	
	if (IID == IID_SIMPLE_ZUMI)
		TicksPerZumiMove = getWorld()->getLev()->getOptionValue(optionTicksPerSimpleZumiMove);
	else
		TicksPerZumiMove = getWorld()->getLev()->getOptionValue(optionTicksPerComplexZumiMove);

	currentTick = TicksPerZumiMove;
}

void Zumi::setDead()
{
	Actor::setDead();
	getWorld()->playSound(SOUND_ENEMY_DIE);
	getWorld()->increaseScore(100);
	getWorld()->deadZumi();

	//Calculate whether to drop Goodie
	int probability = getWorld()->getLev()->getOptionValue(optionProbOfGoodieOverall);
	int number = rand() % 100;
	if (number < probability)
		DropGoodie(getX(), getY());
}

void Zumi::doSomething()
{
	if (!isAlive())
		return;

	//if on same square as player, set player dead
	if (getWorld()->getPlayer()->getX() == getX())
	{
		if (getWorld()->getPlayer()->getY() == getY())
		{
			getWorld()->getPlayer()->setDead();
		}
	}

	currentTick--;
}

void Zumi::move()
{
	switch (returnDirection())
	{
	case KEY_PRESS_LEFT:
	{
						   Actor* leftdirection = getWorld()->WhatActorHere(getX() - 1, getY());
						   Brick* leftbrick = dynamic_cast<Brick*>(leftdirection);
						   BugSprayer* leftbs = dynamic_cast<BugSprayer*>(leftdirection);
						   if (leftbrick || leftbs)
							   resetDirection();
						   else
							   moveTo(getX() - 1, getY());
						   break;
	}
	case KEY_PRESS_RIGHT:
	{
							Actor* rightdirection = getWorld()->WhatActorHere(getX() + 1, getY());
							Brick* rightbrick = dynamic_cast<Brick*>(rightdirection);
							BugSprayer* rightbs = dynamic_cast<BugSprayer*>(rightdirection);
							if (rightbrick || rightbs)
								resetDirection();
							else
								moveTo(getX() + 1, getY());
							break;
	}
	case KEY_PRESS_UP:
	{
						 Actor* updirection = getWorld()->WhatActorHere(getX(), getY() + 1);
						 Brick* upbrick = dynamic_cast<Brick*>(updirection);
						 BugSprayer* upbs = dynamic_cast<BugSprayer*>(updirection);
						 if (upbrick || upbs)
							 resetDirection();
						 else
							 moveTo(getX(), getY() + 1);
						 break;
	}
	case KEY_PRESS_DOWN:
	{
						   Actor* downdirection = getWorld()->WhatActorHere(getX(), getY() - 1);
						   Brick* downbrick = dynamic_cast<Brick*>(downdirection);
						   BugSprayer* downbs = dynamic_cast<BugSprayer*>(downdirection);
						   if (downbrick || downbs)
							   resetDirection();
						   else
							   moveTo(getX(), getY() - 1);
						   break;
	}
	}
	reset();
}

void Zumi::DropGoodie(int x, int y)
{
	int chance[100];
	int count = 0;

	//set up probability array
	for (int k = 0; k < getWorld()->getExtraProb(); k++)
	{
		chance[k] = 0;
	}
	count += getWorld()->getExtraProb();
	for (int k = 0; k < getWorld()->getThroughWallsProb(); k++)
	{
		chance[count + k] = 1;
	}
	count += getWorld()->getThroughWallsProb();
	for (int k = 0; k < getWorld()->getBoosterProb(); k++)
	{
		chance[count + k] = 2;
	}

	//pick number slot in array, return Goodie
	int num = rand() % 100;
	if (chance[num] == 0)
		getWorld()->GoodieDrop(0, x, y);
	else if (chance[num] == 1)
		getWorld()->GoodieDrop(1, x, y);
	else
		getWorld()->GoodieDrop(2, x, y);
}

//SIMPLE ZUMI
void SimpleZumi::doSomething()
{
	Zumi::doSomething();
	if (returnTicks() == 0)
	{
		move();
	}
}

//COMPLEX ZUMI

void ComplexZumi::doSomething()
{
	Zumi::doSomething();
	if (returnTicks() == 0)
	{
		int horDistance = abs(getWorld()->getPlayer()->getX() - getX());
		int verDistance = abs(getWorld()->getPlayer()->getY() - getY());
		int smellDistance = getWorld()->getLev()->getOptionValue(optionComplexZumiSearchDistance);

		int direction = getWorld()->path(getX(), getY(), getWorld()->getPlayer()->getX(), getWorld()->getPlayer()->getY());

		if (horDistance < smellDistance && verDistance < smellDistance && direction != -1)
		{
			changeDirection(direction);
		}

		move();
	}
}

//Goodie

Goodie::Goodie(int IID, int startX, int startY, StudentWorld* world)
	: TimedLifeActor(IID, startX, startY, world)
{
	int ticks = getWorld()->getLev()->getOptionValue(optionGoodieLifetimeInTicks);
	setTicks(ticks);
}

//Extra Life Goodie

void ExtraLifeGoodie::doSomething()
{
	Goodie::doSomething();

	if (getWorld()->getPlayer()->getX() == getX() &&
		getWorld()->getPlayer()->getY() == getY())
	{
		getWorld()->getLives();
		getWorld()->increaseScore(1000);
		getWorld()->playSound(SOUND_GOT_GOODIE);
		setDead();
	}
}

//Walk Through Walls Goodie

void WalkThroughWallsGoodie::doSomething()
{
	Goodie::doSomething();

	if (getWorld()->getPlayer()->getX() == getX() &&
		getWorld()->getPlayer()->getY() == getY())
	{
		getWorld()->getPlayer()->PickUpWalkThroughWallsGoodie();
		getWorld()->increaseScore(1000);
		getWorld()->playSound(SOUND_GOT_GOODIE);
		setDead();
	}
}

//Boost Sprayer Goodie

void BoostSprayerGoodie::doSomething()
{
	Goodie::doSomething();

	if (getWorld()->getPlayer()->getX() == getX() &&
		getWorld()->getPlayer()->getY() == getY())
	{
		getWorld()->getPlayer()->PickUpBoosterSprayerGoodie();
		getWorld()->increaseScore(1000);
		getWorld()->playSound(SOUND_GOT_GOODIE);
		setDead();
	}
}