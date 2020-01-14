//
// Created by obyoxar on 10/01/2020.
//

#pragma once

#include <Brain.hpp>
#include <Data/StrikerAction.hpp>
#include <Data/RobotPosition.hpp>
#include <Data/BallData.hpp>
#include <Data/FieldDimensions.hpp>

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

private:
	const Dependency<FieldDimensions> fieldDimensions_;
	const Dependency<RobotPosition> robotPosition_;
	const Dependency<BallState> ballState_;

	Production<StrikerAction> strikerAction_;
};
