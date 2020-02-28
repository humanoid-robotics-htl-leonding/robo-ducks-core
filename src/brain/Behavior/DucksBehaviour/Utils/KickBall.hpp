#pragma once

/**
 * Automatically walks behind ball and tries to kick into the desired Position
 * @author Simon Holzapfel
 * @param d
 * @param targetPos Where to kick to (absolute coordinates)
 * @return
 */
DucksActionCommand kick(const DucksDataSet& d, const Vector2f& targetPos){

	Vector2f goalToBall = d.teamBallModel.position - targetPos;
	float orientation = std::atan2(-goalToBall.y(), -goalToBall.x());

	Pose ballCoordinateSystem = Pose(d.teamBallModel.position, orientation);
	Pose robotPoseRelativeToBallR = Pose(Vector2f(-0.17, 0.06), 0);
	Pose robotPoseRelativeToBallL = Pose(Vector2f(-0.17, -0.06), 0);

	float distToR = (d.robotPosition.pose.position - robotPoseRelativeToBallR.position).norm();
	float distToL = (d.robotPosition.pose.position - robotPoseRelativeToBallL.position).norm();

	//TODO This does not seem to work. Please investigate.
	Pose robotPoseRelativeToBall = distToR < distToL ? robotPoseRelativeToBallR : robotPoseRelativeToBallL;

	Pose globalTargetRobotPosition = ballCoordinateSystem * robotPoseRelativeToBall;

	Pose currentRobotPosition = d.robotPosition.pose;

	if(currentRobotPosition.isNear(globalTargetRobotPosition, 0.045)) {
		return DucksActionCommand::kick(d.robotPosition.fieldToRobot(d.teamBallModel.position),
										d.robotPosition.fieldToRobot(targetPos));
	}else{
		return DucksActionCommand::walk(
			d.robotPosition.fieldToRobot(globalTargetRobotPosition),
			WalkMode::WALK_BEHIND_BALL);
	}

}