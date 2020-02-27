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

    auto absoluteBallPosition = teamBallModel_->position;
    strikerAction_->valid = true;

    if (robotPosition_->pose.position.x() > 0) {
        if (absoluteBallPosition.x() >= 0 && teamBallModel_->found) {
            Vector2f ball = teamBallModel_->position;
            Vector2f goal = Vector2f(fieldDimensions_->fieldLength/2, 0);
            Vector2f goalToBall =  ball - goal;
            Vector2f goalToBallNormalized = goalToBall.normalized();
            Vector2f ballToRobotDistanceX = goalToBallNormalized * 0.20;
            Vector2f ballToGoal = -goalToBall;
            Vector2f ballToRobotDistanceY = Vector2f(ballToGoal.y(), -ballToGoal.x()).normalized() * 0.07;
            Vector2f kickPosition = ball + ballToRobotDistanceY + ballToRobotDistanceX;
            Vector2f kickPositionToGoal = goal - kickPosition;
            float kickPoseAngle = std::atan2(ballToGoal.y(), ballToGoal.x());
            kickPoseAngle = (kickPoseAngle <0) ? kickPoseAngle + 2* M_PI : kickPoseAngle;

<<<<<<< HEAD
            float rotationAngle = std::atan2(ballToRobotDistanceY.y(), ballToRobotDistanceY.x());
            rotationAngle = (rotationAngle <0) ? rotationAngle + 2* M_PI : rotationAngle;

            Rotation2Df rotationMatrix(rotationAngle);
=======
            Rotation2Df rotationMatrix(-kickPoseAngle);
>>>>>>> duck-striker-provider
            Vector2f rotatedTargetVector = rotationMatrix.toRotationMatrix()*kickPositionToGoal;


            strikerAction_->kickPose = Pose(kickPosition,kickPoseAngle);
            Vector2f ballToRobotDistanceY = Vector2f(ballToGoal.y(), -ballToGoal.x()).normalized() * 0.07;
            Vector2f kickPosition = ball + ballToRobotDistanceY + ballToRobotDistanceX;
            Vector2f kickPositionToGoal = goal - kickPosition;
            float kickPoseAngle = std::atan2(ballToGoal.y(), ballToGoal.x());
            kickPoseAngle = (kickPoseAngle <0) ? kickPoseAngle + 2* M_PI : kickPoseAngle;

            Rotation2Df rotationMatrix(-kickPoseAngle);
            Vector2f rotatedTargetVector = rotationMatrix.toRotationMatrix()*kickPositionToGoal;


            strikerAction_->kickPose = Pose(kickPosition,kickPoseAngle);
            strikerAction_->action = DucksStrikerAction::Action::WALK_TO_POS;
            float difference = (robotPosition_->pose.position - kickPosition).norm();
            float rotationDifference = robotPosition_->pose.orientation - strikerAction_->kickPose.orientation;


            if(difference <= 0.1 && rotationDifference <= 0.1 && ballState_->found) {
                strikerAction_->action = DucksStrikerAction::Action::KICK_INTO_GOAL;
                strikerAction_->kickType = DucksStrikerAction::KickType::KICK;
                strikerAction_->kickable = BallUtils::Kickable::LEFT;
                strikerAction_->target = rotatedTargetVector;
<<<<<<< HEAD
=======

>>>>>>> duck-striker-provider
                strikerAction_->target = rotatedTargetVector;

            }
        } else if (isSurrounded()) {
            // TODO: shoots to the nearest teammate
        } else {
            strikerAction_->valid = true;
            strikerAction_->action = DucksStrikerAction::Action ::WAITING_FOR_BALL;
            strikerAction_->kickType = DucksStrikerAction::KickType::NONE;
            Vector2f robotToBall = teamBallModel_->position - robotPosition_->pose.position;
            strikerAction_->kickPose = Pose(Vector2f(robotPosition_->pose.position.x(), teamBallModel_->position.y()), std::atan2(robotToBall.y(), robotToBall.x()));
            isSurrounded();
        }
    } else if (robotPosition_->pose.position.x() < 0){
        if (ballState_->found) {
            Vector2f robotToBall = teamBallModel_->position - robotPosition_->pose.position;
            float rotationDifference = robotPosition_->pose.orientation - std::atan2(robotToBall.y(), robotToBall.x());
            isSurrounded();

            if(!(robotToBall.norm() <= 0.5 && rotationDifference <= 0.5 && ballState_->found)) {
                strikerAction_->kickPose = Pose(teamBallModel_->position, std::atan2(robotToBall.y(), robotToBall.x()));
                strikerAction_->action = DucksStrikerAction::Action::WALK_TO_POS;
                isSurrounded();
            } else if (robotToBall.norm() <= 0.5 && rotationDifference <= 0.5 && ballState_->found) {
            if (robotToBall.norm() <= 0.2 && rotationDifference <= 0.5 && ballState_->found) {
                //std::cout << "dribble to Goal" << std::endl;
                strikerAction_->action = DucksStrikerAction::Action::DRIBBLE_TO_POS;
                Vector2f robotToGoal = Vector2f(fieldDimensions_->fieldLength / 2, 0) - robotPosition_->pose.position;
                Vector2f goalPos = Vector2f (fieldDimensions_->fieldLength/2, 0);
                strikerAction_->kickPose = Pose(goalPos, std::atan2(robotToGoal.y(), robotToGoal.x()));
                Vector2f goalPos = Vector2f(fieldDimensions_->fieldLength/2, 0) - robotPosition_->pose.position;
                strikerAction_->kickPose = Pose(goalPos, std::atan2(goalPos.y(), goalPos.x()));
            } else {
                //std::cout << "nope" << std::endl;
            }
        }
    }
}

bool DucksStrikerActionProvider::isSurrounded() {
    // TODO: retrun true if surrounded, return false if not
    for (auto teamObstacle : teamObstacleData_->obstacles) {
        if (teamObstacle.type == ObstacleType::HOSTILE_ROBOT || true) {
            //std::cout << "teamObstaclePosition: " << teamObstacle.absolutePosition.x() << ";" << teamObstacle.absolutePosition.y() << std::endl << std::endl;
        }
    }

    return false;
}
