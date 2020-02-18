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
    if(desperation_->lookAtBallUrgency >= 1) {
        strikerAction_->valid = false;
        return;
    }

/*	auto goalPos = Vector2f (fieldDimensions_->fieldLength/2, 0);
	auto ballPos = robotPosition_->robotToField(ballState_->position);

	auto goalToBall = (ballPos - goalPos).normalized();

	auto targetPos = ballPos + (goalToBall * 0.1);
	auto orientation = acos(goalToBall.dot(Vector2f(1.0, 0)));

	debug().update(mount_+ ".yeet", Pose(targetPos, M_PI+orientation));
	debug().update(mount_+ ".goalPos", Pose(goalPos, orientation));
	debug().update(mount_+ ".ballPos", Pose(ballPos, orientation));

	strikerAction_->kickable = BallUtils::Kickable::NOT;
	strikerAction_->kickPose = robotPosition_->fieldToRobot(Pose(targetPos, orientation));
	strikerAction_->type = StrikerAction::Type::KICK_INTO_GOAL;
	strikerAction_->passTarget = 0;
	strikerAction_->target = Vector2f(0, 0);
	strikerAction_->valid = true;*/


    auto absoluteBallPosition = teamBallModel_->position;
    strikerAction_->valid = true;

    // std::cout << "" << absoluteBallPosition.x() << " " << absoluteBallPosition.y() << std::endl;

    if (absoluteBallPosition.x() >= 0 && teamBallModel_->found) {
        Vector2f ball = teamBallModel_->position;
        Vector2f goal = Vector2f(fieldDimensions_->fieldLength/2, 0);
        Vector2f goalToBall =  ball - goal;
        Vector2f goalToBallNormalized = goalToBall.normalized();
        Vector2f ballToRobotDistanceY = goalToBallNormalized * 0.2;
        Vector2f ballToGoal = -goalToBall;
        Vector2f ballToRobotDistanceX = Vector2f(ballToGoal.y(), -ballToGoal.x()).normalized() * 0.07;
        Vector2f kickPosition = goal + goalToBall + ballToRobotDistanceY + ballToRobotDistanceX;
        strikerAction_->kickPose = Pose(kickPosition, std::atan2(goalToBall.y(), ballToGoal.x()));
        strikerAction_->action = DucksStrikerAction::Action::WALK_TO_POS;
        float difference = (robotPosition_->pose.position - kickPosition).norm();
        float rotationDifference = robotPosition_->pose.orientation - strikerAction_->kickPose.orientation;

        if(difference <= 0.1 && rotationDifference <= 0.1 && ballState_->found) {
            strikerAction_->action = DucksStrikerAction::Action::KICK_INTO_GOAL;
            strikerAction_->kickType = DucksStrikerAction::KickType::KICK;
            strikerAction_->kickable = BallUtils::Kickable::LEFT;
        }
    } else if (isSurrounded()) {
        // TODO: shoots to the nearest teammate
    } else {
        strikerAction_->valid = false;
        strikerAction_->action = DucksStrikerAction::Action ::WAITING_FOR_BALL;
        strikerAction_->kickType = DucksStrikerAction::KickType::NONE;
        // TODO: go back to fixed position
    }
}

bool DucksStrikerActionProvider::isSurrounded() {
    for (auto teamObstacle : teamObstacleData_->obstacles) {
        //std::cout << "teamObstaclePosition: " << teamObstacle.absolutePosition.x() << ";" << teamObstacle.absolutePosition.y() << std::endl << std::endl;
    }

    return false;
}
