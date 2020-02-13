
#include <cmath>
#include "Tools/Chronometer.hpp"
#include "Tools/Math/Angle.hpp"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Range.hpp"

#include "DucksDefenderActionProvider.hpp"


DucksDefenderActionProvider::DucksDefenderActionProvider(const ModuleManagerInterface &manager)
	: Module(manager),
	  ballFocalPointDepth_(*this, "ballFocalPointDepth", []
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
	  worldState_(*this),
	  defendingPosition_(*this)
{
}

void DucksDefenderActionProvider::cycle()
{
	Chronometer time(debug(), mount_ + ".cycle_time");


	if (gameControllerState_->gameState == GameState::PLAYING) {
		defendingPosition_->valid = true;

		auto ballX = teamBallModel_->position.x();
		auto ballY = teamBallModel_->position.y();
		auto defenderX = std::min(ballX - 1.0, -fieldDimensions_->fieldLength / 4.);

		auto focalX = -fieldDimensions_->fieldLength / 2. - ballFocalPointDepth_();
		auto focalY = ballFocalPointYshift_();

		auto dX = ballX - focalX;
		auto dY = ballY - focalY;

		auto k = dY / dX;

		auto defenderY = k * (defenderX - focalX) + focalY;

		auto suggestedIntersectPosition = Vector2f(defenderX, defenderY);

		auto ballToSuggested = teamBallModel_->position - suggestedIntersectPosition;

		if (ballToSuggested.norm() < maxDeflectBallDistance_()) {
			defendingPosition_->targetPose =
				Pose(teamBallModel_->position, std::atan2(ballToSuggested.y(), ballToSuggested.x()));
		}
		else {
			defendingPosition_->targetPose.position = Vector2f(defenderX, defenderY);
		}

		Vector2f moveDist = (defendingPosition_->targetPose.position - robotPosition_->pose.position);
		Vector2f scaledMoveDist = moveDist.cwiseProduct(Vector2f(1.0, 0.5));

		if (scaledMoveDist.norm() < 1.0) {
			defendingPosition_->type = DucksDefendingPosition::Type::DEFEND;
		}
		else {
			defendingPosition_->type = DucksDefendingPosition::Type::MOVE;
		}
	}
}