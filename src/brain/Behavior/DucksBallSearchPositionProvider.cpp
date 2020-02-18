#include <Eigen/Dense>

#include "Tools/Chronometer.hpp"
#include "Tools/Math/HungarianMethod.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Range.hpp"

#include "print.h"

#include "DucksBallSearchPositionProvider.hpp"


DucksBallSearchPositionProvider::DucksBallSearchPositionProvider(const ModuleManagerInterface &manager)
	: Module(manager),
	  ballSearchMap_(*this),
	  gameControllerState_(*this),
	  teamPlayers_(*this),
	  ballState_(*this),
	  robotPosition_(*this),
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
	  standingOnCooldown_(0),
	  oldSearchPosition_(0, 0),
	  oldSearchProbability_(0)
{
	maxSideAngle_() *= TO_RAD;
}

void DucksBallSearchPositionProvider::cycle()
{
	Chronometer time(debug(), mount_ + ".cycle_time");



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
		bool thisIsValid = false;
		while(fieldSearchPositionIterator != list.cend()){
			auto positionUnderInspection = (*fieldSearchPositionIterator)->position;
			auto myDistance = (robotPosition_->pose.position - positionUnderInspection).norm();

			//3.1.1 We dont want the position if my team players are nearer or are walking to it.
			thisIsValid = true;
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

			float probabilityIncreaseFactor = (*fieldSearchPositionIterator)->probability / oldSearchProbability_;
			if(probabilityIncreaseFactor < 1.2){
				thisIsValid = false;
			}

			if(thisIsValid) break;
			fieldSearchPositionIterator++;
		}
		auto fieldSearchPosition = (*fieldSearchPositionIterator)->position;
		if(!thisIsValid){
			fieldSearchPosition = oldSearchPosition_;
			oldSearchProbability_ = ballSearchMap_->cellFromPositionConst(fieldSearchPosition).probability;
		}else{
			oldSearchProbability_ = (*fieldSearchPositionIterator)->probability;
		}

		//3.2 Look at snacked position
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

	//2. === If ball rolls to the side, then turn
	if(searchPosition_->reason != DuckBallSearchPosition::Reason::SEARCH_WALK){
		auto localSearchPosition = robotPosition_->fieldToRobot(searchPosition_->searchPosition);
		auto angleToSearchPosition = std::atan2(localSearchPosition.y(), localSearchPosition.x());
		if(std::abs(angleToSearchPosition) > maxSideAngle_()) {
			Vector2f posToRobot = searchPosition_->searchPosition - robotPosition_->pose.position;
			auto angle = std::atan2(posToRobot.y(), posToRobot.x());

			searchPosition_->pose = Pose(robotPosition_->pose.position, angle);
			searchPosition_->reason = DuckBallSearchPosition::Reason::SEARCH_TURN;
		}
	}
	//2. === Step Back
//	auto distVec = teamBallModel_->position - robotPosition_->pose.position;
//	auto dist = distVec.norm();
//	auto distAlert = dist < stepBackThreshold_() && !teamBallModel_->seen;
//
//	if(distAlert || standingOnCooldown_ > 0){
//		standingOnCooldown_ -= cycleInfo_->cycleTime;
//		if(distAlert){
//			standingOnCooldown_ = 1;
//		}
//		searchPosition_->ownSearchPoseValid = true;
//		searchPosition_->reason = DuckBallSearchPosition::Reason::I_AM_ON_IT;
//		searchPosition_->pose = robotPosition_->robotToField(Pose(stepBackValue_(), 0));
//		//TODO Dont run out of field.
//	}

	//Hysteresis
	oldSearchPosition_ = searchPosition_->searchPosition;
}