
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
	  defendThreshold_(*this, "defendThreshold", []
	  {}),
	  kickThreshold_(*this, "kickThreshold", []
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

void DucksDefenderActionProvider::cycle()
{
	Chronometer time(debug(), mount_ + ".cycle_time");

	defenderAction_->valid = false;

	if (gameControllerState_->gameState == GameState::PLAYING) {
		defenderAction_->valid = true;

		const TeamPlayer *keeper = nullptr;
		findKeeper(keeper);

		if (fieldZones_->isInside(teamBallModel_->position, fieldZones_->keeper) && keeper != nullptr) {
			guard();
		} else if (fieldZones_->isInside(teamBallModel_->position, fieldZones_->defenderDribble) ) {
			dribble();
		} else if (fieldZones_->isInside(teamBallModel_->position, fieldZones_->defenderKick)) {
			kick();
		} else {
			defend();
		}
	}
}

void DucksDefenderActionProvider::defend() {
	float fieldLengthHalf = fieldDimensions_->fieldLength / 2.;
	float fieldWidthHalf = fieldDimensions_->fieldWidth / 2.;
	float goalWidthHalf = fieldDimensions_->goalInnerWidth / 2. + fieldDimensions_->goalPostDiameter / 2.;

	auto defenderX = fieldZones_->defenderKick.topLeft.x();

	auto leftGoalPost = Vector2f(-fieldLengthHalf, goalWidthHalf);

	auto dangerZoneBorder = Line<float>(fieldZones_->defenderKick.bottomRight, leftGoalPost);
	auto middleLineLong = Line<float>(
			Vector2f(fieldLengthHalf, 0),
			Vector2f(-fieldLengthHalf, 0));
	auto defenderXLine = Line<float>(
			Vector2f(defenderX, fieldWidthHalf),
			Vector2f(defenderX, -fieldWidthHalf));

	Vector2f focal;
	Geometry::getIntersection(dangerZoneBorder, middleLineLong, focal);

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
	Geometry::getIntersection(ballToFocal, defenderXLine, suggestedIntersectPosition);

	auto ballToSuggested = suggestedIntersectPosition - teamBallModel_->position;

	defenderAction_->targetPose =
			Pose(suggestedIntersectPosition, std::atan2(ballToSuggested.y(), ballToSuggested.x()));

	Vector2f moveDist = (defenderAction_->targetPose.position - robotPosition_->pose.position);

	Vector2f scaledMoveDist = moveDist.cwiseProduct(Vector2f(1.0, 0.5));

	float urgency = moveDist.norm() / (fieldLengthHalf - defenderX);

	if (scaledMoveDist.norm() < defendThreshold_()) {
		defenderAction_->type = DucksDefenderAction::Type::DEFEND;
	}
	else {
		defenderAction_->type = DucksDefenderAction::Type::WALK;
	}

	if (desperation_->lookAtBallUrgency > urgency) {
		defenderAction_->valid = false;
	}
}

void DucksDefenderActionProvider::kick() {
	const TeamPlayer *otherDefender = nullptr;
	findOtherDefender(otherDefender);

	Vector2f ballDist = teamBallModel_->position - robotPosition_->pose.position;

	if (otherDefender != nullptr) {
		auto otherBallDist = teamBallModel_->position - otherDefender->pose.position;
		if (otherBallDist.norm() < otherBallDist.norm()) {
			guard();
			return;
		}
	}

	float urgency = ballDist.norm() / (fieldZones_->defenderKick.topLeft - fieldZones_->defenderKick.bottomRight).norm();

	defenderAction_->type = DucksDefenderAction::Type::KICK;

	defenderAction_->targetPose = Pose(0, robotPosition_->pose.position.y());

	if (desperation_->lookAtBallUrgency > urgency) {
		defenderAction_->valid = false;
	}
}

void DucksDefenderActionProvider::dribble() {
	auto fieldLengthHalf = fieldDimensions_->fieldLength / 2.;

	auto targetPose = Pose(teamBallModel_->position.x() - 0.5, teamBallModel_->position.y());

	Vector2f moveDist = (targetPose.position - robotPosition_->pose.position);

	const TeamPlayer *otherDefender = nullptr;
	findOtherDefender(otherDefender);

	if (otherDefender != nullptr) {
		auto otherMoveDist = (targetPose.position - otherDefender->pose.position);
		if (otherMoveDist.norm() < moveDist.norm()) {
			guard();
			return;
		}
	}

	Vector2f scaledMoveDist = moveDist.cwiseProduct(Vector2f(1.0, 0.5));

	auto goalYProjection = teamBallModel_->position.y() / fieldDimensions_->fieldWidth * fieldDimensions_->goalInnerWidth;
	auto ballToGoal = Vector2f(-fieldLengthHalf, goalYProjection) - teamBallModel_->position;

	float urgency = moveDist.norm() / ballToGoal.norm();

	if (scaledMoveDist.norm() < dribbleThreshold_() ||
		(abs(teamBallModel_->position.y() - robotPosition_->pose.position.y()) < 0.1 && teamBallModel_->position.x() > robotPosition_->pose.position.x())) {
		defenderAction_->targetPose =
				Pose(teamBallModel_->position.x(), teamBallModel_->position.y());
		defenderAction_->type = DucksDefenderAction::Type::WALKDIRECT_WITH_ORIENTATION;
	} else {
		defenderAction_->targetPose = targetPose;
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

	Vector2f moveDist = (targetPose.position - robotPosition_->pose.position);

	const TeamPlayer *otherDefender = nullptr;
	findOtherDefender(otherDefender);

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

	Vector2f scaledMoveDist = moveDist.cwiseProduct(Vector2f(1.0, 0.5));

	const TeamPlayer *keeper = nullptr;
	findKeeper(keeper);

	if (keeper != nullptr) {
		auto keeperToBall = teamBallModel_->position - keeper->pose.position;
		float urgency = keeperToBall.norm() / moveDist.norm();

		if (scaledMoveDist.norm() < defendThreshold_()) {
			defenderAction_->type = DucksDefenderAction::Type::DEFEND;
		}
		else {
			defenderAction_->targetPose = targetPose;
			defenderAction_->type = DucksDefenderAction::Type::WALK;
		}

		if (desperation_->lookAtBallUrgency > urgency) {
			defenderAction_->valid = false;
		}
	} else {
		defenderAction_->valid = false;
	}
}

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