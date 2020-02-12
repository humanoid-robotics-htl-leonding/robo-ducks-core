#pragma once

#include "Data/DucksDefendingPosition.hpp"
#include "Data/FieldDimensions.hpp"
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
	const Parameter<float> ballFocalPointDepth_;
	const Parameter<float> maxDeflectBallDistance_;

	const Dependency<FieldDimensions> fieldDimensions_;
	const Dependency<GameControllerState> gameControllerState_;
	const Dependency<ObstacleData> obstacleData_;
	const Dependency<PlayingRoles> playingRoles_;
	const Dependency<RobotPosition> robotPosition_;
	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<TeamPlayers> teamPlayers_;
	const Dependency<WorldState> worldState_;
	Production<DucksDefendingPosition> defendingPosition_;

};
