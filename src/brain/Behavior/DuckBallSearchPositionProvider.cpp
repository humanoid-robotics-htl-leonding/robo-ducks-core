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
	  maxSideAngle_(*this, "maxSearchSideAngle", [this] { maxSideAngle_() *= TO_RAD; }),
	  searchPosition_(*this),
	  fieldLength_(fieldDimensions_->fieldLength),
	  fieldWidth_(fieldDimensions_->fieldWidth),
	  lookAtQueue_()
{
	maxSideAngle_() *= TO_RAD;
}

void DuckBallSearchPositionProvider::cycle()
{
	{
		Chronometer time(debug(), mount_ + ".cycle_time");

		auto distVec = teamBallModel_->position - robotPosition_->pose.position;
		auto dist = distVec.norm();


		debug().update(mount_+".testPose", Pose(0, 0, 0));
		//1. === Implement "Look At Ball" (Erik Mayrhofer)

		// If we see the ball on our own, look at it.
		if (ballState_->age < 1.0 && ballState_->confident) {
			auto ballPos = ballState_->position + ballState_->velocity*cycleInfo_->cycleTime;
			searchPosition_->searchPosition = robotPosition_->robotToField(ballPos);
			searchPosition_->ownSearchPoseValid = true;
			searchPosition_->reason = DuckBallSearchPosition::Reason::OWN_CAMERA;
			standingOnCooldown_ = 0;
			//If we don't see the ball on our own, look at where our team thinks the ball is. Maybe we can find it there.
		}else if(teamBallModel_->found && teamBallModel_->insideField){
			standingOnCooldown_ = 0;
			auto ballPos = teamBallModel_->position + teamBallModel_->velocity*cycleInfo_->cycleTime;
			searchPosition_->searchPosition = ballPos;
			searchPosition_->ownSearchPoseValid = true;
			searchPosition_->reason = DuckBallSearchPosition::Reason::TEAM_BALL_MODEL;
		}
		//3. == Scan Field
		else{
			auto list = std::list<ProbCell*>(ballSearchMap_->probabilityList_); //Copy
			list.sort(ProbCell::probability_comparator_desc);

			//3.1 === Snack a possible position to investigate
			auto fieldSearchPositionIterator = list.cbegin();
			while(fieldSearchPositionIterator != list.cend()){
				auto positionUnderInspection = (*fieldSearchPositionIterator)->position;
				auto myDistance = (robotPosition_->pose.position - positionUnderInspection).norm();

				//3.1.1 We dont want the position if my team players are nearer or are walking to it.
				bool thisIsValid = true;
				for(auto teamPlayer = teamPlayers_->players.cbegin(); teamPlayer < teamPlayers_->players.cend(); teamPlayer++){

					//Other Team Player is closer
					if((teamPlayer->pose.position - positionUnderInspection).norm() < myDistance){
						thisIsValid = false;
						break;
					}
					//Other Team Player is investigating same are
					//TODO and i am not significantly closer to the area
//					if((teamPlayer->currentSearchPosition - positionUnderInspection).norm() < maxBallDetectionRange_()){
//						thisIsValid = false;
//						break;
//					}
				}

				if(thisIsValid) break;
				fieldSearchPositionIterator++;
			}

			//3.2 Look at snacked position
			auto fieldSearchPosition = (*fieldSearchPositionIterator)->position;
			searchPosition_->searchPosition = fieldSearchPosition;
			searchPosition_->reason = DuckBallSearchPosition::Reason::SEARCHING;
			searchPosition_->ownSearchPoseValid=true;

			//3.3 If position is too far away.... walk to it.
			auto localSearchPosition = robotPosition_->fieldToRobot(searchPosition_->searchPosition);
			if(localSearchPosition.norm() > maxBallDetectionRange_()){
				Vector2f posToRobot = fieldSearchPosition - robotPosition_->pose.position;
				posToRobot.normalize();
				auto targetWalkPos = fieldSearchPosition - posToRobot * inspectBallRange_();
				auto angle = std::atan2(posToRobot.y(), posToRobot.x());

				searchPosition_->pose = Pose(targetWalkPos, angle);
				searchPosition_->reason = DuckBallSearchPosition::Reason::SEARCH_WALK;
			}
		}

		if(searchPosition_->reason != DuckBallSearchPosition::Reason::SEARCH_WALK){
			auto localSearchPosition = robotPosition_->fieldToRobot(searchPosition_->searchPosition);
			auto angleToSearchPosition = std::atan2(localSearchPosition.y(), localSearchPosition.x());
			debug().update(mount_+".angle", angleToSearchPosition/TO_RAD);
			std::cout << angleToSearchPosition << " -- " << maxSideAngle_() << std::endl;
			if(std::abs(angleToSearchPosition) > maxSideAngle_()) {
				Vector2f posToRobot = searchPosition_->searchPosition - robotPosition_->pose.position;
				auto angle = std::atan2(posToRobot.y(), posToRobot.x());

				searchPosition_->pose = Pose(robotPosition_->pose.position, angle);
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




		debug().update(mount_ + ".distance", dist);
		debug().update(mount_ + ".standingOnCooldown", standingOnCooldown_);

	}
}