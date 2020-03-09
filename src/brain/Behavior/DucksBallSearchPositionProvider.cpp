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
	  desperation_(*this),
	  minBallDetectionRange_(*this, "minBallDetectionRange"),
	  maxBallDetectionRange_(*this, "maxBallDetectionRange"),
	  inspectBallRange_(*this, "inspectBallRange"),
	  maxAgeValueContribution_(*this, "maxAgeValueContribution"),
	  probabilityWeight_(*this, "probabilityWeight"),
	  voronoiSeeds_(*this, "voronoiSeeds"),
	  stepBackValue_(*this, "stepBackValue"),
	  stepBackThreshold_(*this, "stepBackThreshold"),
	  maxSideAngle_(*this, "maxSearchSideAngle", [this] { maxSideAngle_() *= TO_RAD; }),
	  comfortableSideAngle_(*this, "comfortableSearchSideAngle", [this] { comfortableSideAngle_() *= TO_RAD; }),
	  minComfortableProbability_(*this, "minComfortableProbability"),
	  minProbability_(*this, "minProbability"),
	  minUncomfortableUrgency_(*this, "minUncomfortableUrgency"),
	  minTurnUrgency_(*this, "minTurnUrgency"),
	  searchPosition_(*this),
	  fieldLength_(fieldDimensions_->fieldLength),
	  fieldWidth_(fieldDimensions_->fieldWidth),
	  standingOnCooldown_(0),
	  oldSearchPosition_()
{
	maxSideAngle_() *= TO_RAD;
    comfortableSideAngle_() *= TO_RAD;
}

void DucksBallSearchPositionProvider::cycle()
{
	searchPosition_->reset();
	Chronometer time(debug(), mount_ + ".cycle_time");

	DuckBallSearchPosition pos;

	bool valid = generateBallSearchPositionWithPolicies(pos);
	if(!valid) Log(LogLevel::WARNING) << "Could not generate valid BallSearchPosition";

	if(desperation_->lookAtBallUrgency < minUncomfortableUrgency_()){ //Don't allow incomfortable Decisions
		if(!iWantToLookAt(pos.searchPosition)){ // But this position is uncomfortable
			Log(LogLevel::WARNING) << "LookAtBallUrgency is not high enough to allow uncomfortable positions, but this position was uncomfortable";
		}
	}

	searchPosition_->ownSearchPoseValid = valid;
	if(valid){
		searchPosition_->pose = pos.pose;
		searchPosition_->reason = pos.reason;
		searchPosition_->searchPosition = pos.searchPosition;
	}



	searchPosition_->pose = robotPosition_->pose;
	auto searchPoseToPos = searchPosition_->searchPosition - searchPosition_->pose.position;
	auto searchPoseToPosAngle = std::atan2(searchPoseToPos.y(), searchPoseToPos.x());
	searchPosition_->pose.orientation = Angle::normalized(searchPoseToPosAngle);



	auto robotOrientation = robotPosition_->pose.orientation;
	auto robotToPoint = searchPosition_->searchPosition - robotPosition_->pose.position;
	auto robotToPointAngle = std::atan2(robotToPoint.y(), robotToPoint.x());
	float angleDiff = std::abs(Angle::angleDiff(robotOrientation, robotToPointAngle));
	debug().update(mount_+".angle", angleDiff);




	//2. === If ball rolls to the side, then turn (if walking... so that if we are already walking, we dont look at death)
//	if(searchPosition_->reason != DuckBallSearchPosition::Reason::SEARCH_WALK){
//		auto localSearchPosition = robotPosition_->fieldToRobot(searchPosition_->searchPosition);
//		auto angleToSearchPosition = std::atan2(localSearchPosition.y(), localSearchPosition.x());
//		if(std::abs(angleToSearchPosition) > maxSideAngle_()) {
//			Vector2f posToRobot = searchPosition_->searchPosition - robotPosition_->pose.position;
//			auto angle = Angle::normalized(std::atan2(posToRobot.y(), posToRobot.x()));
//
//			searchPosition_->pose = Pose(robotPosition_->pose.position, angle);
//			searchPosition_->reason = DuckBallSearchPosition::Reason::SEARCH_TURN;
//		}
//	}

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
//	}
}

bool DucksBallSearchPositionProvider::iWantToLookAt(const Vector2f &point) {

    if(desperation_->lookAtBallUrgency < minUncomfortableUrgency_()) { // Comfortable mode
        return this->robotPosition_->pose.frustrumContainsPoint(point, comfortableSideAngle_());
    }else if (desperation_->lookAtBallUrgency < minTurnUrgency_()) { // Hard mode
        return this->robotPosition_->pose.frustrumContainsPoint(point, maxSideAngle_());
    }else{ //Panic Mode
        return true;
    }

}

ProbCell const* DucksBallSearchPositionProvider::snackPositionToLookAt() {
    auto list = std::list<ProbCell*>(ballSearchMap_->probabilityList_); //Copy
    list.sort(ProbCell::probability_comparator_desc);

    //3.1 === Snack a possible position to investigate
    auto fieldSearchPositionIterator = list.cbegin();
    auto hardSearchPositionIterator = list.cend();
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
                if(!searchPosition_->suggestedSearchPositionValid[teamPlayer->playerNumber]){
					searchPosition_->suggestedSearchPositions[teamPlayer->playerNumber] = positionUnderInspection;
					searchPosition_->suggestedSearchPositionValid[teamPlayer->playerNumber] = true;
				}
                break;
            }
            //TODO Other Team Player is investigating same are and i am not significantly closer to the area
        }

//        float probabilityIncreaseFactor = (*fieldSearchPositionIterator)->probability / oldSearchPosition_->probability;
//        if(probabilityIncreaseFactor < 1.2){
//            thisIsValid = false;
//        }

        if(thisIsValid) {
            if(iWantToLookAt(positionUnderInspection)) {
                break;
            }else{
                if(hardSearchPositionIterator == list.cend()){
                    hardSearchPositionIterator = fieldSearchPositionIterator;
                }
            }

        }
        fieldSearchPositionIterator++;
    }

    if(fieldSearchPositionIterator != list.cend()){
//        debug().update(mount_+".comfortable", (*fieldSearchPositionIterator));
    }
    if(
            fieldSearchPositionIterator == list.cend() ||
            (hardSearchPositionIterator != list.cend() && (*fieldSearchPositionIterator)->probability < minComfortableProbability_())
            ){
        fieldSearchPositionIterator = hardSearchPositionIterator;
    }


	if (thisIsValid && (*fieldSearchPositionIterator)->probability >= minProbability_()) {
		//Then Accept the new position
		oldSearchPosition_ = (*fieldSearchPositionIterator);
	} // Else remain at the old position
	return oldSearchPosition_;
}
bool DucksBallSearchPositionProvider::policyOwnCamera(DuckBallSearchPosition &position)
{
	//== CASE 1 If we see the ball on our own, look at it.
	if(!ballState_->found) return false;
	auto ballPos = ballState_->position + ballState_->velocity*cycleInfo_->cycleTime;
	position.searchPosition = robotPosition_->robotToField(ballPos);
	position.ownSearchPoseValid = true;
	position.reason = DuckBallSearchPosition::Reason::OWN_CAMERA;
	standingOnCooldown_ = 0;
	return true;

}
bool DucksBallSearchPositionProvider::policyTeamModel(DuckBallSearchPosition &position)
{
	if(!teamBallModel_->found) return false;
	//== CASE 2 If we don't see the ball on our own, look at where our team thinks the ball is. Maybe we can find it there.
	standingOnCooldown_ = 0;
	auto ballPos = teamBallModel_->position + teamBallModel_->velocity*cycleInfo_->cycleTime;
	position.searchPosition = ballPos;
	position.ownSearchPoseValid = true;
	position.reason = DuckBallSearchPosition::Reason::TEAM_BALL_MODEL;

	if(!iWantToLookAt(searchPosition_->searchPosition)){
		position.ownSearchPoseValid = false;
	}
	return true;
}
bool DucksBallSearchPositionProvider::policyBallSearchMap(DuckBallSearchPosition &position)
{
	//3. == Scan Field
	//3.1 Get a position to look at
	auto probCell = snackPositionToLookAt();
	if(probCell == nullptr) return false;

	//== CASE 3 We have a valid position on the ballsearchmap to look at.

	//3.2 Look at snacked position
	position.searchPosition = probCell->position;
	position.reason = DuckBallSearchPosition::Reason::SEARCHING;
	position.ownSearchPoseValid=true;

	//3.3 If position is too far away.... walk to it.
	auto localSearchPosition = robotPosition_->fieldToRobot(searchPosition_->searchPosition);
	if(localSearchPosition.norm() > maxBallDetectionRange_()){
		Vector2f posToRobot = probCell->position - robotPosition_->pose.position;
		posToRobot.normalize();
		auto targetWalkPos = probCell->position - posToRobot * inspectBallRange_();
		auto angle = Angle::normalized(std::atan2(posToRobot.y(), posToRobot.x()));

		position.pose = Pose(targetWalkPos, angle);
		position.reason = DuckBallSearchPosition::Reason::SEARCH_WALK;
	}
	return true;
}
bool DucksBallSearchPositionProvider::policyLookAround(DuckBallSearchPosition &position)
{
	position.reason = DuckBallSearchPosition::Reason::LOOK_AROUND;
	double periodDuration = 2000.0;
	double amplitude = 40*TO_RAD;
	double distance = 0.5;
	double time = ((double) TimePoint::getCurrentTime())/periodDuration*2*M_PI;
	double angle = sin(time)*amplitude;

	double x = cos(angle);
	double y = sin(angle);

	position.searchPosition = robotPosition_->robotToField(Vector2f(x*distance, y*distance));
	position.ownSearchPoseValid = true;

	return true;
}
bool DucksBallSearchPositionProvider::generateBallSearchPositionWithPolicies(DuckBallSearchPosition &position)
{
	if(policyOwnCamera(position)) return true;
	if(policyTeamModel(position)) return true;
	if(policyBallSearchMap(position)) return true;
	if(policyLookAround(position)) return true;

	position.ownSearchPoseValid = false;
	return false;
}
