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
        } else {
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
            Vector2f goal = Vector2f(fieldDimensions_->fieldLength / 2, 0);
            Vector2f goalToBall = ballState_->position - goal;
            Vector2f ballToGoal = -goalToBall;
            Vector2f goalToBallNormalized = goalToBall.normalized();
            Vector2f kickPosition = goal + goalToBall + goalToBallNormalized * 0.1;

            const double distanceToChangeAction = 0.2;
            if (robotToBall.norm() > distanceToChangeAction) {
                strikerAction_->kickPose = Pose(kickPosition, std::atan2(ballToGoal.y(), ballToGoal.x()));
                strikerAction_->action = DucksStrikerAction::Action::WALK_TO_POS;
            } else {
                strikerAction_->action = DucksStrikerAction::Action::WALK_TO_POS;
                Vector2f goalPos = Vector2f(fieldDimensions_->fieldLength / 2, 0);
                strikerAction_->kickPose = Pose(goalPos, 0);
            }
        }
    }
}
