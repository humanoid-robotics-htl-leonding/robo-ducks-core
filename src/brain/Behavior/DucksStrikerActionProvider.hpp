//
// Created by obyoxar on 10/01/2020.
//

#pragma once

#include <Brain.hpp>
#include <Data/DucksStrikerAction.hpp>
#include <Data/RobotPosition.hpp>
#include <Data/BallData.hpp>
#include <Data/FieldDimensions.hpp>
#include <Data/TeamObstacleData.hpp>
#include <Data/TeamBallModel.hpp>
#include <Data/Desperation.hpp>

class DucksStrikerActionProvider : public Module<DucksStrikerActionProvider, Brain>
{
public:
	/// the name of this module
	ModuleName name = "DucksStrikerActionProvider";
	/**
	 * @brief StrikerActionProvider initializes members
	 * @param manager a reference to brain
	 */
	explicit DucksStrikerActionProvider(const ModuleManagerInterface& manager);

	void cycle() override;

	bool isSurrounded();

private:
	const Dependency<FieldDimensions> fieldDimensions_;
	const Dependency<RobotPosition> robotPosition_;
	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<TeamObstacleData> teamObstacleData_;
	const Dependency<Desperation> desperation_;
	const Dependency<BallState> ballState_;

	Production<DucksStrikerAction> strikerAction_;
};
