//
// Created by max on 3/9/20.
//

#include <Tools/Chronometer.hpp>
#include "DucksBishopActionProvider.hpp"
DucksBishopActionProvider::DucksBishopActionProvider(const ModuleManagerInterface &manager)
    : Module(manager),
      fieldZones_(*this),
      gameControllerState_(*this),
      robotPosition_(*this),
      teamBallModel_(*this),
      desperation_(*this),
      bishopAction_(*this)
{

}

void DucksBishopActionProvider::cycle()
{
    Chronometer time(debug(), mount_ + ".cycle_time");

    bishopAction_->valid = false;

    //TODO desperation

    if (gameControllerState_->gameState == GameState::PLAYING) {
        bishopAction_->valid = true;

        if (shouldPatrol()){
            patrol();
        } else if (shouldEvadeLiberationStrike() ) {
            evadeLiberationStrike();
        } else if (shouldPass()) {
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
bool DucksBishopActionProvider::shouldPatrol()
{
    return teamBallModel_->position.x() < fieldZones_->bishopPatrolLeft.topLeft.x();
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
            Rectangle<float> insideZone = (fieldZones_->isInside(robotPosition,fieldZones_->bishopPatrolRight)) ?fieldZones_->bishopPatrolRight :fieldZones_->bishopPatrolLeft;

            //when reaching a corner, go to the next one
        }

}
bool DucksBishopActionProvider::shouldEvadeLiberationStrike()
{
    //defender or keeper are kicking ball away
    return false;
}
void DucksBishopActionProvider::evadeLiberationStrike()
{
    //go out of kickPath
    //go where passing or kicking is then achieved faster
}
bool DucksBishopActionProvider::shouldPass()
{
    //check if striker is in front and easy to pass to
    return false;
}
void DucksBishopActionProvider::pass()
{
    // pass to location near striker - better between center and striker
}
bool DucksBishopActionProvider::shouldShadowBall()
{
    //striker is near ball in front of enemy goal
    return false;
}
void DucksBishopActionProvider::shadowBall()
{
    //go to block ray-shaded segment that is biggest when casting rays from ball around
}
bool DucksBishopActionProvider::shouldStrike()
{
    //check if kicking ball into goal is possible
    return false;
}
void DucksBishopActionProvider::strike()
{
    //kick ball into enemy goal
}
float DucksBishopActionProvider::getZoneCornerPatrolOrientation(Vector2f corner,Rectangle<float> zone)
{
    corner.x();
    zone.topLeft.x();
    return 0;
}
