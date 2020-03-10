//
// Created by max on 3/9/20.
//

#include <Tools/Chronometer.hpp>
#include <Data/TeamPlayers.hpp>
#include <Tools/Math/Geometry.hpp>
#include "DucksBishopActionProvider.hpp"
#include "Tools/Math/Circle.hpp"

DucksBishopActionProvider::DucksBishopActionProvider(const ModuleManagerInterface &manager)
    : Module(manager),
      strikerIsOffensiveLine_(*this, "strikerIsOffensiveLine", []
      {}),
      passOffset_(*this, "passOffset"),
      maxPassDistance_(*this, "maxPassDistance"),
      fieldZones_(*this),
      gameControllerState_(*this),
      robotPosition_(*this),
      teamBallModel_(*this),
      desperation_(*this),
      teamPlayers_(*this),
      fieldDimensions_(*this),
      bishopAction_(*this),
      currentlyDoingPassingAction(false)
{
}

void DucksBishopActionProvider::cycle()
{
    Chronometer time(debug(), mount_ + ".cycle_time");

    bishopAction_->valid = false;

    //TODO desperation

    if (gameControllerState_->gameState == GameState::PLAYING) {
        bishopAction_->valid = true;

        if (shouldPatrol()) {
            patrol();
        } else if (shouldEvadeLiberationStrike() ) {
            evadeLiberationStrike();
        } else if (shouldPass() || currentlyDoingPassingAction) {
            pass();
        } else if (shouldShadowBall()) {
            shadowBall();
        } else if (shouldStrike()) {
            strike();
        } else {
            patrol();
        }
    }
}

//region patrol

bool DucksBishopActionProvider::shouldPatrol()
{
    return teamBallModel_->position.x() < fieldZones_->bishopPatrolLeft.topLeft.x() && !shouldEvadeLiberationStrike();
}
void DucksBishopActionProvider::patrol()
{
        bishopAction_->type= DucksBishopAction::Type::PATROL_AREA;
        //get nearest Zone if not already inside one
        Vector2f robotPosition = robotPosition_->pose.position;
        if(!fieldZones_->isInside(robotPosition,fieldZones_->bishopPatrolLeft) &&
        !fieldZones_->isInside(robotPosition,fieldZones_->bishopPatrolRight)){
            Vector2f nearestCorner;
            Vector2f nearestCornerLeftZone = fieldZones_->nearestCorner(robotPosition_->pose.position,fieldZones_->bishopPatrolLeft);
            Vector2f nearestCornerRightZone = fieldZones_->nearestCorner(robotPosition_->pose.position,fieldZones_->bishopPatrolRight);
            float orientation = 0;
            if(std::abs((robotPosition-nearestCornerLeftZone).norm()) <std::abs((robotPosition-nearestCornerRightZone).norm())){
                nearestCorner = nearestCornerLeftZone;
                orientation = getZoneCornerPatrolOrientation(nearestCorner,fieldZones_->bishopPatrolLeft);
            }
            else {
                nearestCorner = nearestCornerRightZone;
                orientation = getZoneCornerPatrolOrientation(nearestCorner,fieldZones_->bishopPatrolRight);

            }
            bishopAction_->targetPose = Pose(nearestCorner,orientation);
        }
        else {
            //when reaching a corner, go to the next one
            Rectangle<float> insideZone = (fieldZones_->isInside(robotPosition,fieldZones_->bishopPatrolRight)) ?fieldZones_->bishopPatrolRight :fieldZones_->bishopPatrolLeft;
            if(robotPosition_->pose.isNear(Pose(insideZone.topLeft,getZoneCornerPatrolOrientation(insideZone.topLeft,insideZone)))){
                patrolTarget = Pose(insideZone.topRight(),getZoneCornerPatrolOrientation(insideZone.topRight(),insideZone));
            }
            if(robotPosition_->pose.isNear(Pose(insideZone.topRight(),getZoneCornerPatrolOrientation(insideZone.topRight(),insideZone)))){
                patrolTarget = Pose(insideZone.bottomRight,getZoneCornerPatrolOrientation(insideZone.bottomRight,insideZone));
            }
            if(robotPosition_->pose.isNear(Pose(insideZone.bottomRight,getZoneCornerPatrolOrientation(insideZone.bottomRight,insideZone)))){
                patrolTarget = Pose(insideZone.bottomLeft(),getZoneCornerPatrolOrientation(insideZone.bottomLeft(),insideZone));
            }
            if(robotPosition_->pose.isNear(Pose(insideZone.bottomLeft(),getZoneCornerPatrolOrientation(insideZone.bottomLeft(),insideZone)))){
                patrolTarget = Pose(insideZone.topLeft,getZoneCornerPatrolOrientation(insideZone.topLeft,insideZone));
            }

            bishopAction_->targetPose = patrolTarget;
        }

}

//endregion

//region evadeLiberationStrike

bool DucksBishopActionProvider::shouldEvadeLiberationStrike()
{
    //defender or keeper are kicking ball away
    return false;
}
void DucksBishopActionProvider::evadeLiberationStrike()
{
    //go out of kickPath
    //go where passing to striker or kicking is then achieved faster
}

//endregion

//region pass

bool DucksBishopActionProvider::shouldPass()
{
    //check if striker is in front and easy to pass to
    //check if ball is in PassZone
    if(fieldZones_->isInside(teamBallModel_->position,fieldZones_->bishopPass)){
        //check if keeper is offensive
        const TeamPlayer *striker = nullptr;
        findStriker(striker);
        if (striker != nullptr) {
            if(!(striker->pose.position.x() >= strikerIsOffensiveLine_())){
                return false;
            }
            //check if Passtarget is easily kicked to
            //get all raySegments facing the ball along the maxPassCircle
            int strikerIsLeft = (striker->pose.position.y() >=0) ? 1:-1;
            Vector2f proposedPassTarget = striker->pose.position + passOffset_() * strikerIsLeft;
            Vector2f ballPosition = teamBallModel_->position;
            Circle<float> passCircle = Circle<float>(proposedPassTarget,maxPassDistance_());
            std::vector<Vector2f> raySegmentCenters = std::vector<Vector2f>();
            float jump =0.0;
            bool foundIntersection;
            //this orders our segments according to distance to current ball position
            do{
                foundIntersection = false;
                std::vector<float> intersections =passCircle.getXIntersections(ballPosition.y()+jump);
                if(!intersections.empty()){
                    foundIntersection = true;
                    if(abs(ballPosition.y()+jump)<=fieldDimensions_->fieldWidth/2.0){
                        raySegmentCenters.emplace_back(intersections.at(0),ballPosition.y()+jump);
                    }
                }
                if(jump == 0){
                    jump = 0.5;
                }
                if(jump >0){
                    jump *=-1.0;
                }
                else {
                    jump = jump *-1.0 +0.5;
                }

            }while(foundIntersection);

            //check if a segment is not blocked s
            for (int i =0;i<(int)raySegmentCenters.size();i++){
                if(segmentToBallIsIntersected(raySegmentCenters.at(i))){
                    passBallSource = raySegmentCenters.at(i);
                    passBallTarget = proposedPassTarget;
                    currentlyDoingPassingAction = true;
                    return true;
                }
            }

        }
    }
    return false;
}
void DucksBishopActionProvider::pass()
{
    //dribble to passing Location
    if(!robotPosition_->pose.isNear(Pose(passBallSource,std::atan((passBallTarget.y()-passBallSource.y())/(passBallTarget.x()-passBallSource.x()))))){
        bishopAction_->targetPose = Pose(passBallSource,std::atan((passBallTarget.y()-passBallSource.y())/(passBallTarget.x()-passBallSource.x())));
        bishopAction_->type = DucksBishopAction::Type::DRIBBLE_TO_KICK_LOCATION;
    }
    else {
        bishopAction_->type = DucksBishopAction::Type::PASS;
        bishopAction_->kickTarget = passBallTarget;
        currentlyDoingPassingAction = false;
    }
    // pass to location near striker - better between center and striker
}

//endregion

//region shadowBall

bool DucksBishopActionProvider::shouldShadowBall()
{
    //striker is near ball in front of enemy goal
    return false;
}
void DucksBishopActionProvider::shadowBall()
{
    //go to block ray-shaded segment that is biggest when casting rays from ball around
}

//endregion

//region strike

bool DucksBishopActionProvider::shouldStrike()
{
    //check if kicking ball into goal is possible
    return false;
}
void DucksBishopActionProvider::strike()
{
    //kick ball into enemy goal
}

//endregion

float DucksBishopActionProvider::getZoneCornerPatrolOrientation(Vector2f corner,Rectangle<float> zone)
{
    if(corner == zone.topLeft){
        return 0.0;
    }
    else if (corner ==zone.topRight()){
        return -M_PI/2.0;
    }
    else if(corner ==zone.bottomRight){
        return M_PI;
    }
    else{
        return M_PI/2.0;
    }
}
void DucksBishopActionProvider::findStriker(const TeamPlayer *&pPlayer)
{
    for (auto& player : teamPlayers_->players)
    {
        if (player.penalized) {
            continue;
        }

        if (player.currentlyPerformingRole == PlayingRole::STRIKER)
        {
            pPlayer = &player;
        }
    }
}
bool DucksBishopActionProvider::segmentToBallIsIntersected(const Vector2f &segment) {
    Line<float> ballRay(teamBallModel_->position, segment);

    /*for(const auto& robot : teamPlayers_->players){
        if(robot.playerNumber == playerConfiguration_->playerNumber) continue;

        if(robot.pose.position.x() < teamBallModel_->position.x()){
            auto distance = Geometry::distPointToLineSegment(ballRay, robot.pose.position);
            if(distance < robotDiameter_()/2){
                return true;
            }
        }
    }*/
    return false;
}