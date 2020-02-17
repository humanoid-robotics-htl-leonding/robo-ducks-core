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
	, strikerAction_(*this)
{

}

void DucksStrikerActionProvider::cycle()
{
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
	strikerAction_->type = DucksStrikerAction::Type::KICK_INTO_GOAL;
	strikerAction_->kickType = DucksStrikerAction::KickType::KICK;
	strikerAction_->target = Vector2f(0, 0);
	strikerAction_->valid = true;*/

    auto absoluteBallPosition = teamBallModel_->position;
    strikerAction_->valid = true;

    // std::cout << "" << absoluteBallPosition.x() << " " << absoluteBallPosition.y() << std::endl;

    if (absoluteBallPosition.x() >= 0) {
        auto ball = teamBallModel_->position;
        auto targetPos = Vector2f(fieldDimensions_->fieldLength/2, 0);
        Vector2f targetToBall =  ball - targetPos;
        targetToBall.normalize();
        targetToBall *= 0.2;
        targetToBall += ball;
        strikerAction_->kickPose = Pose(targetToBall, 0);
        strikerAction_->action = DucksStrikerAction::WALK_TO_BALL;
        float difference = (robotPosition_->pose.position - targetToBall).norm();

        if (difference <= 0.1){
            strikerAction_->action = DucksStrikerAction::KICK_INTO_GOAL;
            strikerAction_->kickType = DucksStrikerAction::KickType::KICK;
        }
    } else if (isSurrounded()) {
        // TODO: shoots to the nearest teammate
    } else {
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
