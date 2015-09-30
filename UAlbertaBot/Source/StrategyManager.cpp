#include "Common.h"
#include "StrategyManager.h"

const std::string StrategyManager::PROTOSS_ZEALOT_RUSH = "PROTOSS_ZEALOT_RUSH";
const std::string StrategyManager::PROTOSS_DARK_TEMPLAR = "PROTOSS_DARK_TEMPLAR";
const std::string StrategyManager::PROTOSS_DRAGOONS = "PROTOSS_DRAGOONS";
const std::string StrategyManager::PROTOSS_DRAGOON_DEFEND = "PROTOSS_DRAGOON_DEFEND";
const std::string StrategyManager::PROTOSS_SCOUT_RUSH = "PROTOSS_SCOUT_RUSH";
const std::string StrategyManager::PROTOSS_CANNON_DEFEND_AND_ZEALOT_RUSH = "PROTOSS_CANNON_DEFEND_AND_ZEALOT_RUSH";


//Terran Strategies

const std::string StrategyManager::TERRAN_MARINE_RUSH = "TERRAN_MARINE_RUSH";

//Zerg Strategies

const std::string StrategyManager::ZERG_ZERGLING_RUSH = "ZERG_ZERGLING_RUSH";


// constructor
StrategyManager::StrategyManager() 
	: firstAttackSent(false)
	, currentStrategy(PROTOSS_ZEALOT_RUSH)
	, changedStrategy()
	, selfRace(BWAPI::Broodwar->self()->getRace())
	, enemyRace(BWAPI::Broodwar->enemy()->getRace())
{
	addStrategies();
	setStrategy();	
}

// get an instance of this
StrategyManager & StrategyManager::Instance() 
{
	static StrategyManager instance;
	return instance;
}

void StrategyManager::addStrategies() 
{
	//Add all of the opening books to our strategies

	strategies[PROTOSS_ZEALOT_RUSH] = "0 0 0 0 1 0 3 3 0 0 4 1 4 4 0 4 4 0 1 4 3 0 1 0 4 0 4 4 4 4 1 0 4 4 4";
	strategies[PROTOSS_DARK_TEMPLAR] = "0 0 0 0 1 0 3 0 7 0 5 0 12 0 13 3 22 22 1 22 22 0 1 0";
	strategies[PROTOSS_DRAGOONS] = "0 0 0 0 1 0 0 3 0 7 0 0 5 0 0 3 8 6 1 6 6 0 3 1 0 6 6 6";
	strategies[PROTOSS_DRAGOON_DEFEND] = "0 0 0 0 1 0 3 0 7 0 5 0 12 0 13 3 22 22 1 22 22 0 1 0 22";

	//strategies[PROTOSS_SCOUT_RUSH]    = "0 0 0 0 1 7 0 0 0 1 2 0 0 0 7 0 0 0 3 1 5 9 1 12 17 17 17 18 18 18 18 18 18 18";
	strategies[PROTOSS_SCOUT_RUSH] =  "0 0 0 0 1 7 0 0 0 1 2 0 0 0 7";
	strategies[PROTOSS_CANNON_DEFEND_AND_ZEALOT_RUSH]= "0 0 0 0 1 9 10 10 3 3 0 0 4 1 4 4 0 4 4 0 1 4 3 0 1 0 4 0 4 4 4 4 1 0 4 4 4";
	
	strategies[TERRAN_MARINE_RUSH] = "0 0 0 0 0 1 0 0 3 0 0 3 0 1 0 4 0 0 0 6";

	strategies[ZERG_ZERGLING_RUSH] = "0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 0 0 0 1 6";


	if (selfRace == BWAPI::Races::Protoss)
	{
		//If we are protoss we want to add all protoss strategies

		usableStrategies.push_back(PROTOSS_DARK_TEMPLAR);
		usableStrategies.push_back(PROTOSS_DRAGOONS);
		usableStrategies.push_back(PROTOSS_ZEALOT_RUSH);
		usableStrategies.push_back(PROTOSS_SCOUT_RUSH);
		usableStrategies.push_back(PROTOSS_CANNON_DEFEND_AND_ZEALOT_RUSH);
		usableStrategies.push_back(PROTOSS_DRAGOON_DEFEND);
	}
	else if (selfRace == BWAPI::Races::Terran)
	{
		usableStrategies.push_back(TERRAN_MARINE_RUSH);
	}
	else if (selfRace == BWAPI::Races::Zerg)
	{
		usableStrategies.push_back(ZERG_ZERGLING_RUSH);
	}


	if (Options::Modules::USING_STRATEGY_IO)
	{
		readResults();
	}
}

//Reads the results of previous games vs this enemy
void StrategyManager::readResults()
{
	//Check if we were provided with a file to check for settings
	struct stat buf;
	if (stat(Options::FileIO::FILE_SETTINGS, &buf) == -1)
	{
		//Default if we weren't
		readDir = "bwapi-data/read/";
		writeDir = "bwapi-data/write/";
	}
	else
	{
		//Otherwise read the directories from there
		std::ifstream f_in(Options::FileIO::FILE_SETTINGS);
		getline(f_in, readDir);
		getline(f_in, writeDir);
		f_in.close();
	}

	//Each enemy has their own file
	std::string readFile = readDir + BWAPI::Broodwar->enemy()->getName() + ".txt";

	// if the file exists (we've seen this enemy before)
	if(stat(readFile.c_str(), &buf) != -1)
	{

		std::ifstream f_in(readFile.c_str());
		std::string line;

		//Loop line by line, file format is STRATEGY WINS LOSSES\n
		while(getline(f_in, line)) {
			
			//Use a string stream for ease of use
			std::stringstream stream(line);
			
			int wins;
			int losses;

			std::string name;
			
			stream >> name;
			stream >> wins;
			stream >> losses;

			results[name] = std::pair<int, int>(wins, losses);
		}

		f_in.close();
	}
}

//Iterates through our map of results and prints them to a file corresponding
//To the enemy name as 
//ID WINS LOSSES\n

void StrategyManager::writeResults()
{
	//Open the file
	std::string writeFile = writeDir + BWAPI::Broodwar->enemy()->getName() + ".txt";
	std::ofstream f_out(writeFile.c_str());

	//Iterate over all results, writing them
	std::map<std::string, std::pair<int, int> >::iterator iter;

    for (iter = results.begin(); iter != results.end(); ++iter) {
		f_out << iter->first << " " << iter->second.first << " " << iter->second.second << "\n";
	}	

	f_out.close();
}

void StrategyManager::setStrategy()
{
	//hardcoded list of opponents that suck against UAlbertaBot's Zealot Rush
	std::string enemyId(BWAPI::Broodwar->enemy()->getName());
	if((enemyId.compare("NUSBot") == 0) ||
	   (enemyId.compare("Nova") == 0)  ||
	   (enemyId.compare("HITA") == 0)  ||
	   (enemyId.compare("CruzBot") == 0) ||
	   (enemyId.compare("Bonjwa") == 0) ||
	   (enemyId.compare("Oritaka") == 0) ||
	   (enemyId.compare("Yarmouk") == 0) ||
	   (enemyId.compare("MaasCraft") == 0) ||
	   (enemyId.compare("Xelnaga") == 0) ||
	   (enemyId.compare("TerranUAB") == 0)) 
	{
		currentStrategy = PROTOSS_ZEALOT_RUSH;
		lastStrategy = currentStrategy;
		return;
	}
	//Specific strategy to counter LetaBot, can still be tweaked
	//as it doesn't always win.
	if(enemyId.compare("LetaBot") == 0)
	{
		currentStrategy = PROTOSS_CANNON_DEFEND_AND_ZEALOT_RUSH;
		lastStrategy = currentStrategy;
		return;
	}

	// if we are using file io to determine strategy, do so
	if (Options::Modules::USING_STRATEGY_IO)
	{
		double bestUCB = -1;
		std::string bestStrategyName;

		std::vector<std::string>::iterator usableIter;

		//Loop through all the strategies we are using vs this opponent
		//Make sure we try all of them before doing UCB
		for (usableIter = usableStrategies.begin(); usableIter != usableStrategies.end(); ++usableIter) 
		{
			//We haven't seen this strategy, let's use it
			if (results.find(*usableIter) == results.end())
			{
				currentStrategy = *usableIter;
				lastStrategy = currentStrategy;
				return;
			}
		}

		//We've tried all the strategies, let's take the most effective one

		bestStrategyName =  PROTOSS_ZEALOT_RUSH;
		std::map<std::string, std::pair<int, int> >::iterator resultsIter;

		for (resultsIter = results.begin(); resultsIter != results.end(); ++resultsIter) 
		{

			double ucb = getUCBValue(resultsIter->first);

			//Select the highest UCB value
			if (ucb > bestUCB)
			{
				bestUCB = ucb;
				bestStrategyName = resultsIter->first;
			}
		}

		//Use the stategy selected by UCB
		currentStrategy = bestStrategyName;
		lastStrategy = currentStrategy;

	}
}

void StrategyManager::onEnd(const bool isWinner)
{
	if (Options::Modules::USING_STRATEGY_IO)
	{
		//Determine the results of the game
		bool win = false;

		// if the game ended before the tournament time limit
		if (BWAPI::Broodwar->getFrameCount() < Options::Tournament::GAME_END_FRAME)
		{
			win = isWinner;
		}

		// otherwise game timed out so use in-game score
		else
		{
			if (getScore(BWAPI::Broodwar->self()) > getScore(BWAPI::Broodwar->enemy()))
			{
				win = true;
			}
		}
		
		//Initialize if we haven't used this strategy before
		if(results.find(currentStrategy) == results.end()){
			results[currentStrategy] = std::pair<int, int>(0,0);
		}

		//If we won, record it, otherwise record a loss
		if(win) {
			results[currentStrategy].first += 1;
		} else {
			results[currentStrategy].second += 1;
		}

		//If we changed strategies because of a loss, record it

		if(currentStrategy.compare(changedStrategy) == 0) {

			//Initialize if we haven't used this strategy before
			if(results.find(changedStrategy) == results.end()){
				results[changedStrategy] = std::pair<int, int>(0,0);
			}

			results[changedStrategy].second += 1;
		}

		writeResults();
	}
}

const double StrategyManager::getUCBValue(const std::string strategy) const
{
	double totalTrials(0);

	std::map<std::string, std::pair<int, int> >::iterator resultsIter;

	for (resultsIter = results.begin(); resultsIter != results.end(); ++resultsIter) 
	{
		totalTrials += resultsIter->second.first + resultsIter->second.second;
	}

	double C		= 0.7;
	double wins		= results[strategy].first;
	double trials	= results[strategy].first + results[strategy].second;

	double ucb = (wins / trials) + C * sqrt(std::log(totalTrials) / trials);

	return ucb;
}

const int StrategyManager::getScore(BWAPI::Player * player) const
{
	return player->getBuildingScore() + player->getKillScore() + player->getRazingScore() + player->getUnitScore();
}

const std::string StrategyManager::getOpeningBook() const
{
	return strategies[currentStrategy];
}

// when do we want to defend with our workers?
// this function can only be called if we have no fighters to defend with
const int StrategyManager::defendWithWorkers()
{
	if (!Options::Micro::WORKER_DEFENSE)
	{
		return false;
	}

	// our home nexus position
	BWAPI::Position homePosition = BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition();;

	// enemy units near our workers
	int enemyUnitsNearWorkers = 0;

	// defense radius of nexus
	int defenseRadius = 300;

	// fill the set with the types of units we're concerned about
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		// if it's a zergling or a worker we want to defend
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			if (unit->getDistance(homePosition) < defenseRadius)
			{
				enemyUnitsNearWorkers++;
			}
		}
	}

	// if there are enemy units near our workers, we want to defend
	return enemyUnitsNearWorkers;
}

// called by combat commander to determine whether or not to send an attack force
// freeUnits are the units available to do this attack
const bool StrategyManager::doAttack(const std::set<BWAPI::Unit *> & freeUnits)
{
	int ourForceSize = (int)freeUnits.size();

	int numUnitsNeededForAttack = 1;

	//attack only when a strong enough force has been produced
	if(currentStrategy == PROTOSS_SCOUT_RUSH){
		numUnitsNeededForAttack = 20;
	}
	else if(currentStrategy == PROTOSS_DRAGOON_DEFEND){
		if(BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) < 7 && !firstAttackSent ){
			return false;
		};
	}

	bool doAttack  = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) >= 1
					|| ourForceSize >= numUnitsNeededForAttack;

	if (doAttack)
	{
		firstAttackSent = true;
	}

	return doAttack || firstAttackSent;
}

const bool StrategyManager::expandProtossZealotRush() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		return false;
	}

	int numNexus =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numZealots =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
	int frame =					BWAPI::Broodwar->getFrameCount();

	// if there are more than 10 idle workers, expand
	if (WorkerManager::Instance().getNumIdleWorkers() > 10)
	{
		return true;
	}

	// 2nd Nexus Conditions:
	//		We have 12 or more zealots
	//		It is past frame 7000
	if ((numNexus < 2) && (numZealots > 12 || frame > 9000))
	{
		return true;
	}

	// 3nd Nexus Conditions:
	//		We have 24 or more zealots
	//		It is past frame 12000
	if ((numNexus < 3) && (numZealots > 24 || frame > 15000))
	{
		return true;
	}

	if ((numNexus < 4) && (numZealots > 24 || frame > 21000))
	{
		return true;
	}

	if ((numNexus < 5) && (numZealots > 24 || frame > 26000))
	{
		return true;
	}

	if ((numNexus < 6) && (numZealots > 24 || frame > 30000))
	{
		return true;
	}

	return false;
}

const MetaPairVector StrategyManager::getBuildOrderGoal()
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
	{
		if (getCurrentStrategy() == PROTOSS_ZEALOT_RUSH)
		{
			return getProtossZealotRushBuildOrderGoal();
		}
		else if (getCurrentStrategy() == PROTOSS_DARK_TEMPLAR)
		{
			return getProtossDarkTemplarBuildOrderGoal();
		}
		else if (getCurrentStrategy() == PROTOSS_DRAGOONS)
		{
			return getProtossDragoonsBuildOrderGoal();
		}
		else if (getCurrentStrategy() == PROTOSS_SCOUT_RUSH)
		{
			return getProtossScoutRushBuildOrderGoal();
		}
		else if (getCurrentStrategy() == PROTOSS_CANNON_DEFEND_AND_ZEALOT_RUSH)
		{
			return getProtossZealotRushBuildOrderGoal();
		}
		else if (getCurrentStrategy() == PROTOSS_DRAGOON_DEFEND)
		{
			return getProtossDragoonsDefendBuildOrderGoal();
		}

		// if something goes wrong, use zealot goal
		return getProtossZealotRushBuildOrderGoal();
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
	{
		return getTerranBuildOrderGoal();
	}
	else
	{
		return getZergBuildOrderGoal();
	}
}

const MetaPairVector StrategyManager::getProtossDragoonsBuildOrderGoal() const
{
		// the goal to return
	MetaPairVector goal;

	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int dragoonsWanted = numDragoons > 0 ? numDragoons + 6 : 2;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 6;

	// Additions for dynamic strategy changes to ensure key opening book elements are complete
	// if we have switched into the strategy from another
	if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Assimilator))
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Assimilator, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Assimilator) >0)
	{
		if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core) >0)
	{
		if (!BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge))
			goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}
	else
	{
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}

		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || numDragoons > 6 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 2));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}

const MetaPairVector StrategyManager::getProtossDarkTemplarBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numDarkTeplar =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int darkTemplarWanted = 0;
	int dragoonsWanted = numDragoons + 6;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 6;

	// Additions for dynamic strategy changes to ensure key opening book elements are complete
	// if we have switched into the strategy from another
	if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Assimilator))
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Assimilator, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Assimilator) >0)
	{
		if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core) >0)
	{
		if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Citadel_of_Adun))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Citadel_of_Adun) >0)
	{
		if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Templar_Archives))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Templar_Archives, 1));
	}

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numDragoons > 0)
	{
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, darkTemplarWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));
	
	return goal;
}

const MetaPairVector StrategyManager::getProtossZealotRushBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numZealots =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int zealotsWanted = numZealots + 8;
	int dragoonsWanted = numDragoons;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 4;

	// Additions for dynamic strategy changes to ensure key opening book elements are complete
	// if we have switched into the strategy from another
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Assimilator) >0)
	{
		if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core) >0)
	{
		if (!BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge))
			goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Assimilator, 1));
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}

	if (numCyber > 0)
	{
		dragoonsWanted = numDragoons + 2;
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot,	zealotsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}


const MetaPairVector StrategyManager::getProtossScoutRushBuildOrderGoal() const {
	// the goal to return
	MetaPairVector goal;
	int numScouts =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Scout);
	int numCitAdun =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Citadel_of_Adun);
	int numStargate =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Stargate);
	int numForge =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Forge);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numGateWay =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Gateway);
	int numCorsairs =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Corsair);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int starGatesWanted = numStargate + 3;
	int scoutsWanted = numScouts + 10;
	int corsairsWanted = numCorsairs + 4;

	if(numGateWay == 0) {
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway, 1));
	}

	if(numCyber == 0) {
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}

	if(numForge == 0) {
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Forge, 1));
	}

	if(numCitAdun == 0) {
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Citadel_of_Adun,1));
	}
	if(numStargate < 3) {
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Stargate, std::min(3, starGatesWanted)));
	}
	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Scout,	std::min(30, scoutsWanted)));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Corsair,	std::min(10, corsairsWanted)));
	return goal;
}

const MetaPairVector StrategyManager:: getProtossDragoonsDefendBuildOrderGoal() const{
	MetaPairVector goal;

	int numDarkTeplar =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int darkTemplarWanted = numDarkTeplar + 2;
	int dragoonsWanted = numDragoons + 6;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 6;

	// Additions for dynamic strategy changes to ensure key opening book elements are complete
	// if we have switched into the strategy from another
	if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Assimilator))
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Assimilator, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Assimilator) >0)
	{
		if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core) >0)
	{
		if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Citadel_of_Adun))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Citadel_of_Adun, 1));
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Citadel_of_Adun) >0)
	{
		if (!BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Templar_Archives))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Templar_Archives, 1));
	}

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numDragoons > 0)
	{
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (numDarkTeplar > 3 && numNexusAll < 3)
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, darkTemplarWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));
	
	return goal;

}

const MetaPairVector StrategyManager::getTerranBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;

	int numMarines =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int numMedics =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Medic);
	int numWraith =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Wraith);

	int marinesWanted = numMarines + 12;
	int medicsWanted = numMedics + 2;
	int wraithsWanted = numWraith + 4;

	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine,	marinesWanted));

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

const MetaPairVector StrategyManager::getZergBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;
	
	int numMutas  =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
	int numHydras  =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk);

	int mutasWanted = numMutas + 6;
	int hydrasWanted = numHydras + 6;

	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, 4));
	//goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Stim_Packs,	1));

	//goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Medic,		medicsWanted));

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

const std::string StrategyManager::getCurrentStrategy()
{
	return currentStrategy;
}

// Accessor function: returns the last strategy that had been in use
const std::string StrategyManager::getLastStrategy() 
{
	return lastStrategy;
}
// Accessor function: set the current strategy to a new strategy as selected by DynamicStrategyManager
void StrategyManager::setCurrentStrategy(std::string new_strategy)
{
	changedStrategy = currentStrategy;
	currentStrategy = new_strategy;

}
// Accessor function: set the last strategy to match the strategy currently in effect
void StrategyManager::setLastStrategy(std::string current_strategy)
{
	lastStrategy = current_strategy;
}