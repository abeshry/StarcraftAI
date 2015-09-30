#pragma once

#include "Common.h"
#include "BWTA.h"
#include "base/BuildOrderQueue.h"
#include "InformationManager.h"
#include "base/WorkerManager.h"
#include "base/StarcraftBuildOrderSearchManager.h"
#include <sys/stat.h>
#include <cstdlib>

#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftData.hpp"

typedef std::pair<int, int> IntPair;
typedef std::pair<MetaType, UnitCountType> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;

class StrategyManager 
{
	StrategyManager();
	~StrategyManager() {}

	std::string					readDir;
	std::string					writeDir;

	//The pair represents wins/losses total
	mutable std::map<std::string, std::pair<int,int>>	results;

	std::vector<std::string>	usableStrategies;
	mutable std::map<std::string, std::string> strategies;

	std::string					currentStrategy;
	std::string					lastStrategy;
	std::string					changedStrategy;

	BWAPI::Race					selfRace;
	BWAPI::Race					enemyRace;

	bool						firstAttackSent;

	void	addStrategies();
	void	setStrategy();
	void	readResults();
	void	writeResults();

	const	int					getScore(BWAPI::Player * player) const;
	const	double				getUCBValue(const std::string) const;
	
	// protoss strategy
	const	bool				expandProtossZealotRush() const;
	const	std::string			getProtossZealotRushOpeningBook() const;
	const	MetaPairVector		getProtossZealotRushBuildOrderGoal() const;
	const	MetaPairVector		getProtossCannonRushBuildOrderGoal() const; //not utilized currently

	const	bool				expandProtossDarkTemplar() const;
	const	std::string			getProtossDarkTemplarOpeningBook() const;
	const	MetaPairVector		getProtossDarkTemplarBuildOrderGoal() const;

	const	bool				expandProtossDragoons() const;
	const	std::string			getProtossDragoonsOpeningBook() const;
	const	MetaPairVector		getProtossDragoonsBuildOrderGoal() const;

	//scout rush strategy 
	const	MetaPairVector		getProtossScoutRushBuildOrderGoal() const;
	const   MetaPairVector		getProtossDragoonsDefendBuildOrderGoal() const;


	const	MetaPairVector		getTerranBuildOrderGoal() const;
	const	MetaPairVector		getZergBuildOrderGoal() const;

	const	MetaPairVector		getProtossOpeningBook() const;
	const	MetaPairVector		getTerranOpeningBook() const;
	const	MetaPairVector		getZergOpeningBook() const;

public:

	//These need to be initialized in cpp with unique ids.

	//Protoss Strategies

	static const std::string PROTOSS_ZEALOT_RUSH;
	static const std::string PROTOSS_DARK_TEMPLAR;
	static const std::string PROTOSS_DRAGOONS;
	static const std::string PROTOSS_DRAGOON_DEFEND;
	static const std::string PROTOSS_SCOUT_RUSH;
	static const std::string PROTOSS_CANNON_DEFEND_AND_ZEALOT_RUSH;


	//Terran Strategies

	static const std::string TERRAN_MARINE_RUSH;

	//Zerg Strategies

	static const std::string ZERG_ZERGLING_RUSH;

	static	StrategyManager &	Instance();

			void				onEnd(const bool isWinner);
	
	const	bool				regroup(int numInRadius);
	const	bool				doAttack(const std::set<BWAPI::Unit *> & freeUnits);
	const	int				    defendWithWorkers();
	const	bool				rushDetected();

	const	std::string			getCurrentStrategy();
	const	std::string			getLastStrategy(); // accessor returns previous strategy 
	void						setCurrentStrategy(std::string new_strategy);	// accessor sets the current strategy
	void						setLastStrategy(std::string current_strategy);	// accessor sets the previous strategy


	const	MetaPairVector		getBuildOrderGoal();
	const	std::string			getOpeningBook() const;
};
