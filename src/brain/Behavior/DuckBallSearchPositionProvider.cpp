#include <Eigen/Dense>

#include "Tools/Chronometer.hpp"
#include "Tools/Math/HungarianMethod.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Range.hpp"

#include "print.h"

#include "DuckBallSearchPositionProvider.hpp"


DuckBallSearchPositionProvider::DuckBallSearchPositionProvider(const ModuleManagerInterface &manager)
	: Module(manager),
	  ballSearchMap_(*this),
	  gameControllerState_(*this),
	  playerConfiguration_(*this),
	  playingRoles_(*this),
	  teamPlayers_(*this),
	  ballState_(*this),
	  robotPosition_(*this),
	  bodyPose_(*this),
	  teamBallModel_(*this),
	  fieldDimensions_(*this),
	  jointSensorData_(*this),
	  cycleInfo_(*this),
	  minBallDetectionRange_(*this, "minBallDetectionRange", []
	  {}),
	  maxBallDetectionRange_(*this, "maxBallDetectionRange", []
	  {}),
	  inspectBallRange_(*this, "inspectBallRange", []
	  {}),
	  maxAgeValueContribution_(*this, "maxAgeValueContribution", []
	  {}),
	  probabilityWeight_(*this, "probabilityWeight", []
	  {}),
	  voronoiSeeds_(*this, "voronoiSeeds", []
	  {}),
	  stepBackValue_(*this, "stepBackValue", []
	  {}),
	  stepBackThreshold_(*this, "stepBackThreshold", []
	  {}),
	  searchPosition_(*this),
	  fieldLength_(fieldDimensions_->fieldLength),
	  fieldWidth_(fieldDimensions_->fieldWidth),
	  lookAtQueue_()
{
}

void DuckBallSearchPositionProvider::cycle()
{
	{
		Chronometer time(debug(), mount_ + ".cycle_time");

		auto distVec = teamBallModel_->position - robotPosition_->pose.position;
		auto dist = distVec.norm();


		//1. === Implement "Look At Ball" (Erik Mayrhofer)

		// If we see the ball on our own, look at it.
		if (ballState_->age < 1.0 && ballState_->confident) {
			auto ballPos = ballState_->position + ballState_->velocity*cycleInfo_->cycleTime;
			searchPosition_->searchPosition = ballPos;
			searchPosition_->ownSearchPoseValid = true;
			searchPosition_->reason = DuckBallSearchPosition::Reason::OWN_CAMERA;
			standingOnCooldown_ = 0;
			//If we don't see the ball on our own, look at where our team thinks the ball is. Maybe we can find it there.
		}else if(teamBallModel_->found && teamBallModel_->insideField){
			standingOnCooldown_ = 0;
			auto ballPos = robotPosition_->fieldToRobot(teamBallModel_->position + teamBallModel_->velocity*cycleInfo_->cycleTime);
			searchPosition_->searchPosition = ballPos;
			searchPosition_->ownSearchPoseValid = true;
			searchPosition_->reason = DuckBallSearchPosition::Reason::TEAM_BALL_MODEL;
		}
		//3. == Scan Field
		else{
			auto list = std::list<ProbCell*>(ballSearchMap_->probabilityList_); //Copy
			list.sort(ProbCell::probability_comparator_desc);
			auto fieldSearchPosition = (*list.cbegin())->position;
			searchPosition_->searchPosition = robotPosition_->fieldToRobot(fieldSearchPosition);
			searchPosition_->reason = DuckBallSearchPosition::Reason::SEARCHING;
			searchPosition_->ownSearchPoseValid=true;
			if(searchPosition_->searchPosition.norm() > maxBallDetectionRange_()){
				Vector2f posToRobot = fieldSearchPosition - robotPosition_->pose.position;
				posToRobot.normalize();
				auto targetWalkPos = fieldSearchPosition - posToRobot * inspectBallRange_();
				searchPosition_->pose = Pose(targetWalkPos, acos(posToRobot.dot(Vector2f(1.0, 0.0))));
				searchPosition_->reason = DuckBallSearchPosition::Reason::SEARCH_WALK;
			}
		}

		//2. === Step Back

		auto distAlert = dist < stepBackThreshold_() && !teamBallModel_->seen;

		if(distAlert || standingOnCooldown_ > 0){
			standingOnCooldown_ -= cycleInfo_->cycleTime;
			if(distAlert){
				standingOnCooldown_ = 1;
			}
			searchPosition_->ownSearchPoseValid = true;
			searchPosition_->reason = DuckBallSearchPosition::Reason::I_AM_ON_IT;
			searchPosition_->pose = robotPosition_->robotToField(Pose(stepBackValue_(), 0));
			//TODO Dont run out of field.
		}

//		ballSearchMap_->probabilityList_



		debug().update(mount_ + ".distance", dist);
		debug().update(mount_ + ".standingOnCooldown", standingOnCooldown_);

	}
}