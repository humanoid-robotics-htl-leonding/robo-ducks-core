#pragma once

#include "Data/DucksDefenderAction.hpp"
#include "Data/Desperation.hpp"
#include "Data/FieldDimensions.hpp"
#include "Data/FieldZones.hpp"
#include "Data/GameControllerState.hpp"
#include "Data/ObstacleData.hpp"
#include "Data/PlayingRoles.hpp"
#include "Data/RobotPosition.hpp"
#include "Data/TeamBallModel.hpp"
#include "Data/TeamPlayers.hpp"
#include "Data/WorldState.hpp"
#include "Framework/Module.hpp"


class Brain;

class DucksDefenderActionProvider: public Module<DucksDefenderActionProvider, Brain>
{
public:
	/// the name of this module
	ModuleName name = "DucksDefenderActionProvider";
	/**
	 * @brief DucksDefenderActionProvider initializes members
	 * @param manager a reference to brain
	 */
	DucksDefenderActionProvider(const ModuleManagerInterface &manager);
	/**
	 * @brief cycle calculates the defending position
	 */
	void cycle();

private:
	void defend();
	void kick();
	void dribble();
	void guard();
	void findOtherDefender(const TeamPlayer*& otherDefender) const;
	void findKeeper(const TeamPlayer*& keeper) const;

	const Parameter<float> doubleDefenderFocalY_;
	const Parameter<float> guardY_;
	const Parameter<float> defendThreshold_;
	const Parameter<float> kickThreshold_;
	const Parameter<float> dribbleThreshold_;

	const Dependency<FieldDimensions> fieldDimensions_;
	const Dependency<FieldZones> fieldZones_;
	const Dependency<GameControllerState> gameControllerState_;
	const Dependency<ObstacleData> obstacleData_;
	const Dependency<PlayingRoles> playingRoles_;
	const Dependency<RobotPosition> robotPosition_;
	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<TeamPlayers> teamPlayers_;
	const Dependency<Desperation> desperation_;
	const Dependency<WorldState> worldState_;
	Production<DucksDefenderAction> defenderAction_;

};
