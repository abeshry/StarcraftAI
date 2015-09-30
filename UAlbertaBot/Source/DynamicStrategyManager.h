#pragma once

#include "Common.h"
#include "BWTA.h"

#include "UnitData.h"

#include "..\..\SparCraft\source\SparCraft.h"

// DynamicStrategyManager evaluates enemy units and buildings
// to determine the enemy build order and strategy, and changes the current
// strategy based on the evaluation. The class is a singleton that allows frame by
// frame updates triggered by Game Commander timer events. The strategy information is
// passed back and forth between DynamicStrategyManger and StrategyManager, which has no
// update functionality, giving the ability to change strategy on the fly.
// Also refer to: code in ProductionManager that performs the new build order search

class DynamicStrategyManager
{
	DynamicStrategyManager();

	UnitData							enemyUnitData;
	UnitData							selfUnitData;
	std::map<BWAPI::UnitType, int>  	getUnitTypeCountMap(UnitData raceUnitData);
	int									numSwitches;
	std::map<BWAPI::UnitType, int>		enemyTypeCountMap;
	

public:
	//DynamicStrategyManager();
	//~DynamicStrategyManager();
	static DynamicStrategyManager &		Instance();
	void								update();
	void								onStart();
	void								searchNewStrategy();
	
};

