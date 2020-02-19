#include <cmath>

#include "Tools/Chronometer.hpp"
#include "Tools/Math/Angle.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Range.hpp"
#include "Tools/Math/Line.hpp"
#include "Tools/Math/Geometry.hpp"
#include "Tools/PermissionManagement.hpp"

#include "DucksKeeperActionProvider.hpp"

DucksKeeperActionProvider::DucksKeeperActionProvider(const ModuleManagerInterface& manager)
      : Module(manager)
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
{
}

void DucksKeeperActionProvider::cycle() {
    Chronometer time(debug(), mount_ + ".cycle_time");
    if (gameControllerState_->gameState != GameState::PLAYING) {
        return;
    }

    float robotRadius = 0.15;

    float goalX = -fieldDimensions_->fieldLength / 2;
//    auto goalUpperPost = fieldDimensions_->goalInnerWidth / 2;
    auto goalLowerPost = -fieldDimensions_->goalInnerWidth / 2;
    auto segmentCount = static_cast<int>(std::ceil(fieldDimensions_->goalInnerWidth / fieldDimensions_->ballDiameter));
    auto segmentWidth = fieldDimensions_->goalInnerWidth / segmentCount;

    auto goalShadow = std::vector<bool>(segmentCount);

    for(int i = 0; i < segmentCount; i++){
        double segmentCenter = segmentWidth * (i + 0.5) + goalLowerPost;
        
        Line<float> ballRay(teamBallModel_->position, Vector2f(goalX, segmentCenter));

        for(const auto& robot : teamPlayers_->players){
            if(robot.playerNumber == playerConfiguration_->playerNumber) continue;

            if(robot.pose.position.x() < teamBallModel_->position.x()){
                auto distance = Geometry::distPointToLineSegment(ballRay, robot.pose.position);
                if(distance < robotRadius){
                    goalShadow[i] = true;
                    break;
                }
            }
        }
    }

    int largestSegment = 0;
    int largestSegmentBegin = 0;
    int currSegBegin = -1;

    for(int i = 0; i < segmentCount; i++){
        if(!goalShadow[i]) { //I am in a segment
            if (currSegBegin < 0) { //I am entering a Segment
                currSegBegin = i;
            }
        }else{ //I am out of a segment
            if(currSegBegin >= 0){ //I have just left a segment
                int currentLength = i-currSegBegin;
                largestSegment = std::max(currentLength, largestSegment);
                largestSegmentBegin = currSegBegin;
                currSegBegin = -1;
            }
        }
    }
    if(currSegBegin >= 0){ //I have just left a segment
        int currentLength = segmentCount-currSegBegin;
        if(currentLength > largestSegment){
            largestSegment = currentLength;
            largestSegmentBegin = currSegBegin;
        }
        currSegBegin = -1;
    }

    debug().update(mount_ + ".largestSegment", largestSegment);
    debug().update(mount_ + ".largestSegmentBegin", largestSegmentBegin);

    double lSegLen = segmentWidth * largestSegment;
    double lSegBeg = segmentWidth * largestSegmentBegin;

    auto suggestedKeeperPosition = Vector2f(goalX, lSegBeg + lSegLen/2. + goalLowerPost);

    keeperAction_->action = KeeperAction::Action(KeeperAction::Type::BLOCK_GOAL, Pose(suggestedKeeperPosition, 0));

    //f t t f f t t f f f f f

    debug().update(mount_ + ".shadow", goalShadow);
}