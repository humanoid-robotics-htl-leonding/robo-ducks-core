//
// Created by obyoxar on 10/01/2020.
//

#include "DucksStrikerActionProvider.hpp"

DucksStrikerActionProvider::DucksStrikerActionProvider(const ModuleManagerInterface &manager)
	: Module(manager)
	, fieldDimensions_(*this)
	, robotPosition_(*this)
	, teamBallModel_(*this)
    , teamObstacleData_(*this)
    , desperation_(*this)
    , ballState_(*this)
	, strikerAction_(*this)
{

}

void DucksStrikerActionProvider::cycle()
{
    if (desperation_->lookAtBallUrgency >= 1) {
        strikerAction_->valid = false;
        return;
    }

    auto absoluteBallPosition = teamBallModel_->position;
    strikerAction_->valid = true;

    if (robotPosition_->pose.position.x() >= 0) { // is in enemy half
        if (absoluteBallPosition.x() >= 0 && teamBallModel_->found) {
            Vector2f goal = Vector2f(fieldDimensions_->fieldLength / 2, 0);

            strikerAction_->kickType = DucksStrikerAction::KickType::KICK;
            strikerAction_->kickable = BallUtils::LEFT;
            strikerAction_->target = goal;
            strikerAction_->action = DucksStrikerAction::Action::KICK_INTO_GOAL;
        }
        else if (isSurrounded()) {
            // TODO: shoots to the nearest teammate
        }
        else {
            strikerAction_->valid = true;
            strikerAction_->action = DucksStrikerAction::Action::WAITING_FOR_BALL;
            strikerAction_->kickType = DucksStrikerAction::KickType::NONE;
            Vector2f robotToBall = teamBallModel_->position - robotPosition_->pose.position;
            strikerAction_->kickPose = Pose(Vector2f(robotPosition_->pose.position.x(), teamBallModel_->position.y()),
                                            std::atan2(robotToBall.y(), robotToBall.x()));
        }
    } else {
        if (ballState_->found) {
            Vector2f robotToBall = teamBallModel_->position - robotPosition_->pose.position;
            float rotationDifference = robotPosition_->pose.orientation - std::atan2(robotToBall.y(), robotToBall.x());

            const int distanceToChangeAction = 1;
            if (!(robotToBall.norm() <= distanceToChangeAction && rotationDifference <= 0.5)) { // robotToBall.norm() >  1 || rotationDifference > 0.5
                strikerAction_->kickPose = Pose(teamBallModel_->position, std::atan2(robotToBall.y(), robotToBall.x()));
                strikerAction_->action = DucksStrikerAction::Action::WALK_TO_POS;
                isSurrounded();
            } else {
                strikerAction_->action = DucksStrikerAction::Action::DRIBBLE_TO_POS;
                Vector2f goalPos = Vector2f(fieldDimensions_->fieldLength / 2, 0);
                strikerAction_->kickPose = Pose(goalPos, 0);
            }
        }
    }
}
bool DucksStrikerActionProvider::isSurrounded() {
    // TODO: retrun true if surrounded, return false if not
    for (auto teamObstacle : teamObstacleData_->obstacles) {
        if (teamObstacle.type == ObstacleType::HOSTILE_ROBOT || true) {
>        }
    }

    return false;
}
