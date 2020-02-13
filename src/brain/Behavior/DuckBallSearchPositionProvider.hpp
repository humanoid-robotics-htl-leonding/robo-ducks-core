#pragma once

#include <list>
#include <vector>
#include <Data/DuckBallSearchPosition.hpp>
#include <Data/BallState.hpp>
#include <queue>

#include "Framework/Module.hpp"

#include "Data/BallSearchMap.hpp"
#include "Data/BodyPose.hpp"
#include "Data/CycleInfo.hpp"
#include "Data/FieldDimensions.hpp"
#include "Data/GameControllerState.hpp"
#include "Data/JointSensorData.hpp"
#include "Data/PlayerConfiguration.hpp"
#include "Data/PlayingRoles.hpp"
#include "Data/RobotPosition.hpp"
#include "Data/TeamBallModel.hpp"
#include "Data/TeamPlayers.hpp"

class Brain;

class DuckBallSearchPositionProvider: public Module<DuckBallSearchPositionProvider, Brain>
{
public:
	/// the name of this module
	ModuleName name = "DuckBallSearchPositionProvider";

	/**
	 * @brief DuckBallSearchPositionProvider The constructor
	 * @param manager Reference to the ModuleManagerInterface (e.g. brain)
	 */
	explicit DuckBallSearchPositionProvider(const ModuleManagerInterface &manager);

	void cycle() override;

private:
	// All dependencies for this module

	const Dependency<BallSearchMap> ballSearchMap_;
	const Dependency<GameControllerState> gameControllerState_;
	const Dependency<TeamPlayers> teamPlayers_;
	const Dependency<BallState> ballState_;
	const Dependency<RobotPosition> robotPosition_;
	const Dependency<TeamBallModel> teamBallModel_;
	const Dependency<FieldDimensions> fieldDimensions_;
	const Dependency<JointSensorData> jointSensorData_;
	const Dependency<CycleInfo> cycleInfo_;

	// TODO: These two parameters are needed by both, MapManager and PositionProvider.
	/// The minimum distance to a ball search position (you can not find a ball when you are standing
	/// on it)
	const Parameter<float> minBallDetectionRange_;
	/// The range on which it is likely to detect a ball.
	const Parameter<float> maxBallDetectionRange_;
	/// If we suspect a specific ball position, how close shall we go to it.
	const Parameter<float> inspectBallRange_;
	/// The maximum value the age of a cell can contribute to its value.
	const Parameter<float> maxAgeValueContribution_;
	/// The weight applied to a cells probability when calculating the value.
	const Parameter<float> probabilityWeight_;
	/// The voronoi seeds used to divide the field into search areas.
	const Parameter<std::vector<std::vector<Vector2f>>> voronoiSeeds_;
	const Parameter<Vector2f> stepBackValue_;
	const Parameter<float> stepBackThreshold_;

	Parameter<float> desperationMax_;
	Parameter<float> desperationThreshold_;
	Parameter<float> desperationReduction_;

	//Maximum angle the search position may have to the nao without turning
	Parameter<float> maxSideAngle_;

	/// The position to look for a ball.
	Production<DuckBallSearchPosition> searchPosition_;

	/// Field length in m
	const float fieldLength_;
	/// Field with in m
	const float fieldWidth_;

	float standingOnCooldown_;

	float desperation_ = 0;

	Vector2f oldSearchPosition_;
	float oldSearchProbability_;
};
