#pragma once

#include <list>
#include <vector>
#include <Data/DuckBallSearchPosition.hpp>
#include <Data/BallState.hpp>
#include <queue>
#include <Data/Desperation.hpp>

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

class DucksBallSearchPositionProvider: public Module<DucksBallSearchPositionProvider, Brain>
{
public:
	/// the name of this module
	ModuleName name = "DucksBallSearchPositionProvider";

	/**
	 * @brief DucksBallSearchPositionProvider The constructor
	 * @param manager Reference to the ModuleManagerInterface (e.g. brain)
	 */
	explicit DucksBallSearchPositionProvider(const ModuleManagerInterface &manager);

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
	const Dependency<Desperation> desperation_;

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

	// Maximum angle the search position may have to the nao without turning
	Parameter<float> maxSideAngle_;
	// Maxmum angle that is comfortable to look at (E.g without obstructing bottom camera with shoulders)
	Parameter<float> comfortableSideAngle_;
    // Minimum Probability that a comfortable cell must have to still be accepted (if no comfortable cells over this value are found, then switch to uncomfortable cells)
	Parameter<float> minComfortableProbability_;
	// Maximum value of lookAtBallUrgency to allow comfortable decisions
	Parameter<float> minProbability_;
	// Maximum value of lookAtBallUrgency to allow comfortable decisions
	Parameter<float> maxComfortableUrgency_;
	// Maximum value of lookAtBallUrgency to allow decisions without turning
	Parameter<float> maxNoTurnUrgency_;

	/// The position to look for a ball.
	Production<DuckBallSearchPosition> searchPosition_;

	/// Field length in m
	const float fieldLength_;
	/// Field with in m
	const float fieldWidth_;

	float standingOnCooldown_;

    ProbCell const* oldSearchPosition_;

	bool iWantToLookAt(const Vector2f& point);

    ProbCell const* snackPositionToLookAt();
};
