//
// Created by obyoxar on 10/01/2020.
//

#include "DucksStrikerActionProvider.hpp"

DucksStrikerActionProvider::DucksStrikerActionProvider(const ModuleManagerInterface &manager)
	: Module(manager)
	, fieldDimensions_(*this)
	, robotPosition_(*this)
	, ballState_(*this)
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

}