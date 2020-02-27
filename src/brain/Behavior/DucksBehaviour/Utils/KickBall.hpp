#pragma once

/**
 * Automatically walks behind ball and tries to kick into the desired Position
 * @author Simon Holzapfel
 * @param d
 * @param targetPos Where to kick to
 * @return
 */
DucksActionCommand kick(const DuckDataSet& d, const Vector2f& targetPos){

	Vector2f goalToBall = d.teamBallModel.position - targetPos;
	float orientation = std::atan2(-goalToBall.y(), -goalToBall.x());

	Pose ballCoordinateSystem = Pose(d.teamBallModel.position, orientation);
	Pose robotPoseRelativeToBall = Pose(Vector2f(-0.15, 0.04), 0);

	Pose globalTargetRobotPosition = ballCoordinateSystem * robotPoseRelativeToBall;

	Pose currentRobotPosition = d.robotPosition.pose;

	if(currentRobotPosition.isNear(globalTargetRobotPosition)) {
		return DucksActionCommand::kick(d.robotPosition.fieldToRobot(d.teamBallModel.position),
										d.robotPosition.fieldToRobot(targetPos));
	}else{
		return DucksActionCommand::walk(
			d.robotPosition.fieldToRobot(globalTargetRobotPosition),
			WalkMode::WALK_BEHIND_BALL);
	}

}