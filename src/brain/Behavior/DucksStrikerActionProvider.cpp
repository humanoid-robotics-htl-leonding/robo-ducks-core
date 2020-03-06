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
    auto absoluteBallPosition = teamBallModel_->position;
    Vector2f goal = Vector2f(fieldDimensions_->fieldLength / 2, 0);

    if (robotPosition_->pose.position.x() >= 0) { // is in enemy half
        if (absoluteBallPosition.x() >= 0 && teamBallModel_->found) {
            kick(goal);
        } else {
            wait();
        }
    } else {
        if (ballState_->found) {
            dribble(goal);
        }
    }

    if (desperation_->lookAtBallUrgency >= 1) {
        strikerAction_->valid = false;
        return;
    }
}

void DucksStrikerActionProvider::dribble(const Vector2f& target) {
    strikerAction_->walkTarget = Pose(target, 0);
    strikerAction_->action = DucksStrikerAction::DRIBBLE_TO_POS;
    strikerAction_->valid = true;
}

void DucksStrikerActionProvider::kick(const Vector2f& target) {
    strikerAction_->action = DucksStrikerAction::Action::KICK_INTO_GOAL;
    strikerAction_->kickType = DucksStrikerAction::KickType::NONE;
    strikerAction_->target = target;
    strikerAction_->valid = true;
}

void DucksStrikerActionProvider::wait() {
    Vector2f robotToBall = teamBallModel_->position - robotPosition_->pose.position;
    strikerAction_->action = DucksStrikerAction::Action::WAITING_FOR_BALL;
    strikerAction_->kickType = DucksStrikerAction::KickType::NONE;
    strikerAction_->walkTarget = Pose(Vector2f(robotPosition_->pose.position.x(),
            teamBallModel_->position.y()), std::atan2(robotToBall.y(), robotToBall.x()));
    strikerAction_->valid = false;
}
