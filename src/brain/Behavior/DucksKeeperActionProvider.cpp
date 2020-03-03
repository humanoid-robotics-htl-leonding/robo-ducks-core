#include <cmath>

#include "Tools/Chronometer.hpp"
#include "Tools/Math/Angle.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Range.hpp"
#include "Tools/Math/Geometry.hpp"
#include "Tools/PermissionManagement.hpp"

#include "DucksKeeperActionProvider.hpp"

DucksKeeperActionProvider::DucksKeeperActionProvider(const ModuleManagerInterface& manager)
      : Module(manager)
      , shadowCastSpeed_(*this, "shadowCastSpeed", []{})
      , shadowResolveSpeed_(*this, "shadowResolveSpeed", []{})
      , robotDiameter_(*this, "robotDiameter")
      , segmentCount_(*this, "segmentCount", [this]{
          goalShadow_.clear();
          goalShadow_.resize(segmentCount_());
      })
      , keeperBallKickDistance_(*this, "keeperBallKickDistance", []{})
      , fieldZones_(*this)
      , cycleInfo_(*this)
      , ballState_(*this)
      , fieldDimensions_(*this)
      , gameControllerState_(*this)
      , robotPosition_(*this)
      , teamBallModel_(*this)
      , teamPlayers_(*this)
      , worldState_(*this)
      , playerConfiguration_(*this)

      , keeperAction_(*this)
      , keeperPosition_(Vector2f((-fieldDimensions_->fieldLength * 0.5f) +
                                     fieldDimensions_->fieldPenaltyAreaLength * 0.5f,
                                 0.f))
      , goalShadow_(segmentCount_())
{
}

void DucksKeeperActionProvider::cycle() {
    //TODO:
    // * Hysterese für die Proposed Position
    Chronometer time(debug(), mount_ + ".cycle_time");
    if (gameControllerState_->gameState != GameState::PLAYING) {
        return;
    }


    if(ballInKickRange()){
    	keeperAction_->action = KeeperAction::Action(KeeperAction::Type::KICK_AWAY);
    	return;
    }


    //=== 1 Cast shadows on the Goal (Ball is the light source and robots obstruct its light)
    float goalX = -fieldDimensions_->fieldLength / 2;
    auto goalLowerPost = -fieldDimensions_->goalInnerWidth / 2;
    float segmentWidth = fieldDimensions_->goalInnerWidth / segmentCount_();

    for(int i = 0; i < segmentCount_(); i++){
        double segmentCenter = segmentWidth * (i + 0.5) + goalLowerPost;

        if(robotIntersectsRayToSegment(Vector2f(goalX, segmentCenter))) {
            goalShadow_[i] = std::min(1.f, goalShadow_[i]+shadowCastSpeed_());
        }else{
            goalShadow_[i] = std::max(0.f, goalShadow_[i]-shadowResolveSpeed_());
        }

    }

    //=== 2 Filter out Segments where the goal is lit
    proposedPositions_.clear();

    int largestSegment = 0;
    int largestSegmentBegin = 0;
    int currSegBegin = -1;

    for(int i = 0; i < segmentCount_() + 1; i++){ //Overrun array to exit the last segment
        if(i < segmentCount_() && goalShadow_[i] < 0.2) { //I am in a segment
            if (currSegBegin < 0) { //I am entering a Segment
                currSegBegin = i;
            }
        }else{ //I am out of a segment
            if(currSegBegin >= 0){ //I have just left a segment
                int currentLength = i-currSegBegin;
//                largestSegment = std::max(currentLength, largestSegment);
//                largestSegmentBegin = currSegBegin;

                double lSegLen = segmentWidth * currentLength;
                double lSegBeg = segmentWidth * currSegBegin + goalLowerPost;

                // ==== 2.a And then calculate the optimal keeper position for this segment;
                calculateBestKeeperPositionFor(Vector2f(goalX, lSegBeg), Vector2f(goalX, lSegBeg + lSegLen / 2.));

                currSegBegin = -1;
            }
        }
    }

    // === Lastly get the position with the highest score.
    auto bestPosition = std::max_element(proposedPositions_.cbegin(), proposedPositions_.cend(), [](const ProposedPosition& a, const ProposedPosition& b){return a.score < b.score;});

    keeperAction_->action = KeeperAction::Action(KeeperAction::Type::BLOCK_GOAL, Pose(bestPosition->position, 0));

    // === kick when ball in range
    //todo:
    // * add range to parameters
    // * find fitting default maxDistanceToBall
    // * kick ball in correct direction

    debug().update(mount_ + ".largestSegment", largestSegment);
    debug().update(mount_ + ".proposed", proposedPositions_);
    debug().update(mount_ + ".shadow", goalShadow_);
    debug().update(mount_ + ".largestSegmentBegin", largestSegmentBegin);
}



bool DucksKeeperActionProvider::aimingForMyGoal(float orientation){
    if(orientation == 5.0){
        return false;
    }
    return false;
}

bool DucksKeeperActionProvider::ballInKickRange(){
    //todo:
    // * ball in range
    // * aiming for ball


    auto maxDistanceToBall = keeperBallKickDistance_(); //float
    auto ballPos = teamBallModel_->position; //Vector2f
    auto playerPos = robotPosition_->pose.position; //Vector2f

    auto distanceBetweenBallAndPlayer = (ballPos - playerPos).norm();

    debug().update(mount_+".distance", distanceBetweenBallAndPlayer);
    debug().update(mount_+".maxBall", maxDistanceToBall);

	return distanceBetweenBallAndPlayer < maxDistanceToBall;

}

void DucksKeeperActionProvider::calculateBestKeeperPositionFor(const Vector2f &segmentLowerPoint, const Vector2f &segmentMiddlePoint) {
    Vector2f ball = teamBallModel_->position;

    float segmentWidth = (segmentMiddlePoint - segmentLowerPoint).norm()*2;

    //Cast one ray through the end of the segment and one through the middle.
    //We will then shift our position on the middle line, until our boundary circle touches the ray through the segmend end.

    Line<float> lowerline(ball, segmentLowerPoint);
    Line<float> centerLine(ball, segmentMiddlePoint);

    float angle = 0;
    Geometry::getAngleBetween(lowerline, centerLine, angle);


    float neededDistance = (robotDiameter_()/2.f)/std::sin(angle);

    auto ballToGoalie = (segmentMiddlePoint - teamBallModel_->position).normalized();
    auto ballToGoalieScaled = ballToGoalie * neededDistance;

    Vector2f proposedPosition = ballToGoalieScaled + teamBallModel_->position;

    Vector2f truePosition = {0, 0};
    Vector2f nope = {0, 0};
    int intersections = Geometry::getIntersection(fieldZones_->keeper, Line<float>(proposedPosition, Vector2f(fieldZones_->keeper.topLeft.x()+0.01, segmentMiddlePoint.y())), truePosition, nope);
    if(intersections > 0){
    	assert(intersections == 1);
    	proposedPosition = truePosition;
    }

    proposedPositions_.emplace_back(proposedPosition, segmentWidth);
}

bool DucksKeeperActionProvider::robotIntersectsRayToSegment(const Vector2f &segment) {
    Line<float> ballRay(teamBallModel_->position, segment);

    for(const auto& robot : teamPlayers_->players){
        if(robot.playerNumber == playerConfiguration_->playerNumber) continue;

        if(robot.pose.position.x() < teamBallModel_->position.x()){
            auto distance = Geometry::distPointToLineSegment(ballRay, robot.pose.position);
            if(distance < robotDiameter_()/2){
                return true;
            }
        }
    }
    return false;
}


