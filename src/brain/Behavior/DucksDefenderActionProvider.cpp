
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
	  defaultDefenderX_(*this, "defaultDefenderX", []
	  {}),
	  ballFocalPointYshift_(*this, "ballFocalPointYShift", []
	  {}),
	  maxDeflectBallDistance_(*this, "maxDeflectBallDistance", []
	  {}),
	  fieldDimensions_(*this),
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

	defenderAction_->valid = true;

	if (true || gameControllerState_->gameState == GameState::PLAYING) {
		if (teamBallModel_->position.x() >= defaultDefenderX_() + maxDeflectBallDistance_()) {
			defend();
		} else if (teamBallModel_->position.x() >= defaultDefenderX_()) {
			kick();
		} else {
			dribble();
		}
	}
}

void DucksDefenderActionProvider::defend() {
	auto fieldLengthHalf = fieldDimensions_->fieldLength / 2.;
	auto fieldWidthHalf = fieldDimensions_->fieldWidth / 2.;
	auto goalWidthHalf = fieldDimensions_->goalInnerWidth / 2. + fieldDimensions_->goalPostDiameter / 2.;

	auto defenderX = std::min<float>(teamBallModel_->position.x() - 0.5, defaultDefenderX_());

	auto leftDangerZoneBorderEnd = Vector2f(defenderX, fieldWidthHalf);
	auto leftGoalPost = Vector2f(-fieldLengthHalf, goalWidthHalf);

	auto dangerZoneBorder = Line<float>(leftDangerZoneBorderEnd, leftGoalPost);
	auto middleLineLong = Line<float>(
			Vector2f(fieldLengthHalf, 0),
			Vector2f(-fieldLengthHalf, 0));
	auto defenderXLine = Line<float>(
			Vector2f(defenderX, fieldWidthHalf),
			Vector2f(defenderX, -fieldWidthHalf));

	Vector2f focal;
	Geometry::getIntersection(dangerZoneBorder, middleLineLong, focal);
	focal.y() = ballFocalPointYshift_();

	auto ballToFocal = Line<float>(teamBallModel_->position, focal);

	Vector2f suggestedIntersectPosition;
	Geometry::getIntersection(ballToFocal, defenderXLine, suggestedIntersectPosition);

	auto ballToSuggested = teamBallModel_->position - suggestedIntersectPosition;

	defenderAction_->targetPose =
			Pose(suggestedIntersectPosition, std::atan2(ballToSuggested.y(), ballToSuggested.x()));

	Vector2f moveDist = (defenderAction_->targetPose.position - robotPosition_->pose.position);
	Vector2f scaledMoveDist = moveDist.cwiseProduct(Vector2f(1.0, 0.5));

	float urgency = std::min(0., moveDist.norm() / (fieldLengthHalf - defenderX));

	if (scaledMoveDist.norm() < 1.0) {
		defenderAction_->type = DucksDefenderAction::Type::DEFEND;
	}
	else {
		defenderAction_->type = DucksDefenderAction::Type::MOVEDIRECT;
	}

	if (desperation_->lookAtBallUrgency > urgency) {
		defenderAction_->valid = false;
	}
}

void DucksDefenderActionProvider::kick() {
	auto targetX = teamBallModel_->position.x() - 0.15;
	auto targetY = teamBallModel_->position.y() <= robotPosition_->pose.position.y() ?
				   teamBallModel_->position.y() + 0.07 :
				   teamBallModel_->position.y() - 0.07;
	defenderAction_->targetPose = Pose(targetX, targetY);

	Vector2f moveDist = (defenderAction_->targetPose.position - robotPosition_->pose.position);
	Vector2f scaledMoveDist = moveDist.cwiseProduct(Vector2f(1.0, 0.5));

	float urgency = moveDist.squaredNorm() /
			  (fieldDimensions_->fieldWidth * fieldDimensions_->fieldWidth +
			   maxDeflectBallDistance_() * maxDeflectBallDistance_());

	if (scaledMoveDist.norm() < 0.05) {
		defenderAction_->type = DucksDefenderAction::Type::KICK;
	} else {
		defenderAction_->type = DucksDefenderAction::Type::MOVE;
	}

	if (desperation_->lookAtBallUrgency > urgency) {
		defenderAction_->valid = false;
	}
}

void DucksDefenderActionProvider::dribble() {
	auto fieldLengthHalf = fieldDimensions_->fieldLength / 2.;

	auto targetPose = Pose(teamBallModel_->position.x() - 0.5, teamBallModel_->position.y());

	Vector2f moveDist = (targetPose.position - robotPosition_->pose.position);
	Vector2f scaledMoveDist = moveDist.cwiseProduct(Vector2f(1.0, 0.5));

	auto goalYProjection = teamBallModel_->position.y() / fieldDimensions_->fieldWidth * fieldDimensions_->goalInnerWidth;
	auto ballToGoal = Vector2f(-fieldLengthHalf, goalYProjection) - teamBallModel_->position;

	flaot urgency = moveDist.norm() / ballToGoal.norm();

	if (scaledMoveDist.norm() < 0.1 ||
		(abs(teamBallModel_->position.y() - robotPosition_->pose.position.y()) < 0.1 && teamBallModel_->position.x() > robotPosition_->pose.position.x())) {
		defenderAction_->targetPose =
				Pose(teamBallModel_->position.x(), teamBallModel_->position.y());
		defenderAction_->type = DucksDefenderAction::Type::MOVEDIRECT;
	} else {
		defenderAction_->targetPose = targetPose;
		defenderAction_->type = DucksDefenderAction::Type::MOVE;
	}

	if (desperation_->lookAtBallUrgency > urgency) {
		defenderAction_->valid = false;
	}
}