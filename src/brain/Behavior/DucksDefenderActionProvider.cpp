#include <cmath>
#include "Tools/Chronometer.hpp"
#include "Tools/Math/Angle.hpp"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Geometry.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Line.hpp"
#include "Tools/Math/Range.hpp"

#include "DucksDefenderActionProvider.hpp"


DucksDefenderActionProvider::DucksDefenderActionProvider(const ModuleManagerInterface &manager)
	: Module(manager),
	  doubleDefenderFocalY_(*this, "doubleDefenderFocalY", []
	  {}),
	  guardY_(*this, "guardY", []
	  {}),
	  dribbleThreshold_(*this, "dribbleThreshold", []
	  {}),
	  fieldDimensions_(*this),
	  fieldZones_(*this),
	  gameControllerState_(*this),
	  obstacleData_(*this),
	  playingRoles_(*this),
	  robotPosition_(*this),
	  teamBallModel_(*this),
	  teamPlayers_(*this),
	  desperation_(*this),
	  worldState_(*this),
	  defenderAction_(*this)
{
}

/*
is always the first called method
We look at valid again: false => other module more important
After that the zone in which the Defender is will be located and one of the if-Statements will be called.
*/
void DucksDefenderActionProvider::cycle()
{
	Chronometer time(debug(), mount_ + ".cycle_time");

	defenderAction_->valid = false;

	if (gameControllerState_->gameState == GameState::PLAYING) {
		// now valid is true, so its the most important module
		defenderAction_->valid = true;

        // The Defender is looking for a Keeper nearby
		const TeamPlayer *keeper = nullptr;
		findKeeper(keeper);

        //when theres no keeper near and he is in the "keeperZone" the Defender guards
		if (fieldZones_->isInside(teamBallModel_->position, fieldZones_->keeper) && keeper != nullptr) {
			guard();
			// if hes in the "DribbleZone" he dribbles (you can indicate that when you look at fieldZones_->[...]
		} else if (fieldZones_->isInside(teamBallModel_->position, fieldZones_->defenderDribble) ) {
			dribble();
			// if hes in the "KickZone" he kicks
		} else if (fieldZones_->isInside(teamBallModel_->position, fieldZones_->defenderKick)) {
			kick();
			// if none of the above statements are true, he defends
		} else {
			defend();
		}
	}
}


void DucksDefenderActionProvider::defend() {
	float fieldLengthHalf = fieldDimensions_->fieldLength / 2.;
	float fieldWidthHalf = fieldDimensions_->fieldWidth / 2.;
	float goalWidthHalf = fieldDimensions_->goalInnerWidth / 2. + fieldDimensions_->goalPostDiameter / 2.;
        // you get the x coordinate of the zone
	auto defenderX = fieldZones_->defenderKick.topLeft.x();

	auto leftGoalPost = Vector2f(-fieldLengthHalf, goalWidthHalf);
        // calcualte the lines, that will be used later to orientate
	auto dangerZoneBorder = Line<float>(fieldZones_->defenderKick.bottomRight, leftGoalPost);
	auto middleLineLong = Line<float>(
			Vector2f(fieldLengthHalf, 0),
			Vector2f(-fieldLengthHalf, 0));
	auto defenderXLine = Line<float>(
			Vector2f(defenderX, fieldWidthHalf),
			Vector2f(defenderX, -fieldWidthHalf));

	Vector2f focal;
	Geometry::getIntersection(dangerZoneBorder, middleLineLong, focal);

        // search for other teamplayers preferably defenders
	const TeamPlayer *otherDefender = nullptr;
	findOtherDefender(otherDefender);

	if (otherDefender != nullptr) {
		focal.y() = otherDefender->pose.position.y() < robotPosition_->pose.position.y() ?
					doubleDefenderFocalY_() :
					-doubleDefenderFocalY_();
	} else {
		focal.y() = 0;
	}

	auto ballToFocal = Line<float>(teamBallModel_->position, focal);

	Vector2f suggestedIntersectPosition;
        // calculates intersection
	Geometry::getIntersection(ballToFocal, defenderXLine, suggestedIntersectPosition);
        // suggest Ball position is calculated with Intersect
	auto suggestedToBall = teamBallModel_->position - suggestedIntersectPosition;

	defenderAction_->targetPose =
			Pose(suggestedIntersectPosition, std::atan2(suggestedToBall.y(), suggestedToBall.x()));
	defenderAction_->type = DucksDefenderAction::Type::WALK;

	Vector2f moveDist = (defenderAction_->targetPose.position - robotPosition_->pose.position);

	float urgency = moveDist.norm() / (fieldLengthHalf - defenderX);

	if (desperation_->lookAtBallUrgency > urgency) {
		defenderAction_->valid = false;
	}
}

void DucksDefenderActionProvider::kick() {
	const TeamPlayer *otherDefender = nullptr;
	findOtherDefender(otherDefender);

	Vector2f ballDist = teamBallModel_->position - robotPosition_->pose.position;

        // is there another defender you can kick the ball to
	if (otherDefender != nullptr) {
		auto otherBallDist = teamBallModel_->position - otherDefender->pose.position;
		if (otherBallDist.norm() < otherBallDist.norm()) {
			guard();
			return;
		}
	}

	float urgency = ballDist.norm() / (fieldZones_->defenderKick.topLeft - fieldZones_->defenderKick.bottomRight).norm();
        // type of the DucksDefenderAction is set
	defenderAction_->type = DucksDefenderAction::Type::KICK;
        // position of the robot is set
	defenderAction_->targetPose = Pose(0, robotPosition_->pose.position.y());

	if (desperation_->lookAtBallUrgency > urgency) {
		defenderAction_->valid = false;
	}
}

void DucksDefenderActionProvider::dribble() {
	auto fieldLengthHalf = fieldDimensions_->fieldLength / 2.;
        // position where the robot should dribble to
	auto targetPose = Pose(teamBallModel_->position.x() - 0.15, teamBallModel_->position.y());

	Vector2f moveDist = (targetPose.position - robotPosition_->pose.position);

        // if defender of other team is near => guard()
	const TeamPlayer *otherDefender = nullptr;
	findOtherDefender(otherDefender);

	if (otherDefender != nullptr) {
		auto otherMoveDist = (targetPose.position - otherDefender->pose.position);
		if (otherMoveDist.norm() < moveDist.norm()) {
			guard();
			return;
		}
	}
        // position of Goal is calculated
	auto goalYProjection = teamBallModel_->position.y() / fieldDimensions_->fieldWidth * fieldDimensions_->goalInnerWidth;
        auto ballToGoal = Vector2f(-fieldLengthHalf, goalYProjection) - teamBallModel_->position;

	float urgency = moveDist.norm() / ballToGoal.norm();

	defenderAction_->targetPose = targetPose;

	if (defenderAction_->targetPose.isNear(robotPosition_->pose, dribbleThreshold_())) {
		defenderAction_->targetPose = Pose(fieldZones_->defenderKick.bottomRight.x(), teamBallModel_->position.y());
		// DucksDefenderAction type is set, so the robot walks directly to the targetPose
                defenderAction_->type = DucksDefenderAction::Type::WALKDIRECT_WITH_ORIENTATION;
	}
	else {
               //DucksDefenderAction type is set to just walking
		defenderAction_->type = DucksDefenderAction::Type::WALK;
	}

	if (desperation_->lookAtBallUrgency > urgency) {
		defenderAction_->valid = false;
	}
}

void DucksDefenderActionProvider::guard() {
	Pose targetPose(teamBallModel_->position);
	bool ballOnLeft = robotPosition_->pose.position.y() < teamBallModel_->position.y();
	targetPose.position.y() = ballOnLeft ?
			teamBallModel_->position.y() - guardY_() :
			teamBallModel_->position.y() + guardY_();
	targetPose.orientation = ballOnLeft ?
			M_PI / 4 :
			-M_PI / 4;
        // Vector between targetPosition and robotPosition is calculated
	Vector2f moveDist = (targetPose.position - robotPosition_->pose.position);

	const TeamPlayer *otherDefender = nullptr;
	findOtherDefender(otherDefender);

        // which of the defender is the nearest to the ball
	if (otherDefender != nullptr ||
		(!fieldZones_->isInside(teamBallModel_->position, fieldZones_->defenderDribble) &&
		 !fieldZones_->isInside(teamBallModel_->position, fieldZones_->defenderKick))) {
		if ((otherDefender->pose.position.y() < teamBallModel_->position.y() && ballOnLeft) ||
			(otherDefender->pose.position.y() > teamBallModel_->position.y() && !ballOnLeft)) {
			auto otherMoveDist = (targetPose.position - otherDefender->pose.position);
			if (otherMoveDist.norm() < moveDist.norm()) {
				targetPose.position.x() = fieldZones_->defenderKick.bottomRight.x();
				targetPose.orientation = 0;
			}
		}
	}

	const TeamPlayer *keeper = nullptr;
	findKeeper(keeper);

        // if a keeper is found => urgency and the distance from the ball is calculated (defender)
        // if the distance is to big => the keeper will take care of the ball
	if (keeper != nullptr) {
                // keeper should go to ball (psotion of the ball is specified in teamBallModel)
		auto keeperToBall = teamBallModel_->position - keeper->pose.position;
		float urgency = keeperToBall.norm() / moveDist.norm();

		defenderAction_->targetPose = targetPose;
		defenderAction_->type = DucksDefenderAction::Type::WALK;

		if (desperation_->lookAtBallUrgency > urgency) {
			defenderAction_->valid = false;
		}
	} else {
		defenderAction_->valid = false;
	}
}

// if a player with the role of Defender is punished,
// another robot on the team is sought to take on the role of Defender.
// The current role is set to Defender.
void DucksDefenderActionProvider::findOtherDefender(const TeamPlayer*& otherDefender) const
{
	for (auto& player : teamPlayers_->players)
	{
		if (player.penalized) {
			continue;
		}

		if (player.currentlyPerformingRole == PlayingRole::DEFENDER)
		{
			otherDefender = &player;
		}
	}
}

// if a player with the role of Keeper is punished,
// another robot on the team is sought to take on the role of Keeper.
// The current role is set to Keeper.
void DucksDefenderActionProvider::findKeeper(const TeamPlayer*& keeper) const
{
	for (auto& player : teamPlayers_->players)
	{
		if (player.penalized) {
			continue;
		}

		if (player.currentlyPerformingRole == PlayingRole::KEEPER || player.currentlyPerformingRole == PlayingRole::REPLACEMENT_KEEPER)
		{
			keeper = &player;
		}
	}
}