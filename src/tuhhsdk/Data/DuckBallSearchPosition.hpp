#pragma once

#include <vector>

#include "Definitions/RoboCupGameControlData.h"
#include "Framework/DataType.hpp"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Pose.hpp"


struct DuckBallSearchPosition: public DataType<DuckBallSearchPosition>
{
public:
	/**
	 * @brief BallSearchPosiiton initializes members
	 */
	DuckBallSearchPosition()
	{
		for (auto &currSearchPosition : suggestedSearchPositions) {
			currSearchPosition = Vector2f::Zero();
		}
		for (auto &searchPositionValidity : suggestedSearchPositionValid) {
			searchPositionValidity = false;
		}
	};

	enum Reason
	{
		OWN_CAMERA = 0,
		TEAM_BALL_MODEL = 1,
		I_AM_ON_IT = 2,
		SEARCHING = 3,
		SEARCH_WALK = 4,
		SEARCH_TURN = 5,
		LOOK_AROUND = 6
	};

	/// the name of this DataType
	DataTypeName name = "DuckBallSearchPosition";
	/// the pose to move the robot to. (Field coordinates)
	Pose pose;
	/// the position to look at to find the ball. (Field coordinates)
	Vector2f searchPosition = Vector2f::Zero();
	/// the positions to look at to find the ball for all other players.
	std::array<Vector2f, MAX_NUM_PLAYERS> suggestedSearchPositions;
	/// If pose and searchPosition is valid.
	bool ownSearchPoseValid = false;
	/// If the suggestedSearchPosition is valid
	/// (per player; maps to the suggestedSearchPositions array).
	std::array<bool, MAX_NUM_PLAYERS> suggestedSearchPositionValid;
	/// If the robot is available for searching for the ball (info for other players)
	bool availableForSearch = false;
	/// The robot with the oldest, continously updated map (calculated on this robot)
	unsigned int localMostWisePlayerNumber = 0;
	/// The robot with the oldest, continously updated map (which we got from the robot with the
	/// smallest player number)
	unsigned int globalMostWisePlayerNumber = 0;

	Reason reason = Reason::OWN_CAMERA;

	void reset() override
	{
		for (auto &suggestedSearchPosition : suggestedSearchPositions) {
			suggestedSearchPosition.setZero();
		}
		for (auto &valid : suggestedSearchPositionValid) {
			valid = false;
		}
		ownSearchPoseValid = false;
		availableForSearch = false;
	}

	void toValue(Uni::Value &value) const override
	{
		value = Uni::Value(Uni::ValueType::OBJECT);
		value["pose"] << pose;
		value["searchPosition"] << searchPosition;
		value["ownSearchPoseValid"] << ownSearchPoseValid;
		value["availableForSearch"] << availableForSearch;
		value["localMostWisePlayerNumber"] << localMostWisePlayerNumber;
		value["globalMostWisePlayerNumber"] << globalMostWisePlayerNumber;
		value["reason"] << static_cast<int>(reason);
		value["suggestedSearchPositions"] << suggestedSearchPositions;
		value["suggestedSearchPositionsValid"] << suggestedSearchPositionValid;
	}

	void fromValue(const Uni::Value &value) override
	{
		int readNumber;
		value["pose"] >> pose;
		value["searchPosition"] >> searchPosition;
		value["ownSearchPoseValid"] >> ownSearchPoseValid;
		value["availableForSearch"] >> availableForSearch;
		value["localMostWisePlayerNumber"] >> localMostWisePlayerNumber;
		value["globalMostWisePlayerNumber"] >> globalMostWisePlayerNumber;
		value["reason"] >> readNumber;
		reason = static_cast<Reason>(readNumber);
		value["suggestedSearchPositionsValid"] >> suggestedSearchPositionValid;
		value["suggestedSearchPositions"] >> suggestedSearchPositions;
	}
};
