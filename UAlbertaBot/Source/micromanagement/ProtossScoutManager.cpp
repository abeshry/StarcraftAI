#include "Common.h"
#include "ProtossScoutManager.h"
#include "micromanagement/MicroManager.h"

ProtossScoutManager::ProtossScoutManager() { }

void ProtossScoutManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & scoutUnits = getUnits();
	if (scoutUnits.empty())
	{
		return;
	}
	// figure out targets
	UnitVector scoutUnitTargets;
	for (size_t i(0); i<targets.size(); i++) 
	{
		// conditions for targeting
		if (targets[i]->isVisible()) 
		{
			scoutUnitTargets.push_back(targets[i]);
		}
	}

	// for each scout
	BOOST_FOREACH(BWAPI::Unit * scoutUnit, scoutUnits)
	{
		// if the order is to attack or defend
		if (order.type == order.Attack || order.type == order.Defend) {

			// if there are targets
			if (!scoutUnitTargets.empty())
			{
				// find the best target for this meleeUnit
				BWAPI::Unit * target = getTarget(scoutUnit, scoutUnitTargets);
				// attack it
				smartAttackUnit(scoutUnit, target);
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (scoutUnit->getDistance(order.position) > 100)
				{
					// move to it
					smartMove(scoutUnit, order.position);
				}
			}
		}
	}
}

// get a target for the scout to attack
BWAPI::Unit * ProtossScoutManager::getTarget(BWAPI::Unit * rangedUnit, UnitVector & targets)
{
	int range(rangedUnit->getType().groundWeapon().maxRange());

	int highestInRangePriority(0);
	int highestNotInRangePriority(0);
	int lowestInRangeHitPoints(10000);
	int lowestNotInRangeDistance(10000);

	BWAPI::Unit * inRangeTarget = NULL;
	BWAPI::Unit * notInRangeTarget = NULL;

	BOOST_FOREACH(BWAPI::Unit * unit, targets)
	{
		int priority = getAttackPriority(rangedUnit, unit);
		int distance = rangedUnit->getDistance(unit);

		// if the unit is in range, update the target with the lowest hp
		if (rangedUnit->getDistance(unit) <= range)
		{
			if (priority > highestInRangePriority ||
				(priority == highestInRangePriority && unit->getHitPoints() < lowestInRangeHitPoints))
			{
				lowestInRangeHitPoints = unit->getHitPoints();
				highestInRangePriority = priority;
				inRangeTarget = unit;
			}
		}
		// otherwise it isn't in range so see if it's closest
		else
		{
			if (priority > highestNotInRangePriority ||
				(priority == highestNotInRangePriority && distance < lowestNotInRangeDistance))
			{
				lowestNotInRangeDistance = distance;
				highestNotInRangePriority = priority;
				notInRangeTarget = unit;
			}
		}
	}

	// if there is a highest priority unit in range, attack it first
	return (highestInRangePriority >= highestNotInRangePriority) ? inRangeTarget : notInRangeTarget;
}

	// get the attack priority of a type
int ProtossScoutManager::getAttackPriority(BWAPI::Unit * rangedUnit, BWAPI::Unit * target) 
{
	BWAPI::UnitType rangedUnitType = rangedUnit->getType();
	BWAPI::UnitType targetType = target->getType();

	bool canAttackUs = rangedUnitType.isFlyer() ? targetType.airWeapon() != BWAPI::WeaponTypes::None : targetType.groundWeapon() != BWAPI::WeaponTypes::None;

	// Highest priority are Carriers. We don't want to fight the Interceptors
	if (targetType == BWAPI::UnitTypes::Protoss_Carrier) 
	{
		return 4;
	} 
	// second highest priority is something that can attack us or aid in combat
	if (targetType == BWAPI::UnitTypes::Terran_Medic || canAttackUs ||
		targetType ==  BWAPI::UnitTypes::Terran_Bunker) 
	{
		return 3;
	} 
	// next priority is worker
	else if (targetType.isWorker()) 
	{
		return 2;
	} 
	// then everything else
	else 
	{
		return 1;
	}
}

BWAPI::Unit * ProtossScoutManager::closestrangedUnit(BWAPI::Unit * target, std::set<BWAPI::Unit *> & scoutUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit * closest = NULL;

	BOOST_FOREACH (BWAPI::Unit * rangedUnit, scoutUnitsToAssign)
	{
		double distance = rangedUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = rangedUnit;
		}
	}
	
	return closest;
}