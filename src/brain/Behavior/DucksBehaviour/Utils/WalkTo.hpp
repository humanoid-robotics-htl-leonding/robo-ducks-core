//
// Created by obyoxar on 10/01/2020.
//
#pragma once

DucksActionCommand walkTo(const Pose &fieldPose, const DucksDataSet &d, const WalkMode mode = WalkMode::DIRECT)
{
	auto localPose = d.robotPosition.fieldToRobot(fieldPose);
	if (!d.robotPosition.pose.isNear(fieldPose)) { //todo HYSCHTEREESSCHHEEE
		return DucksActionCommand::walk(localPose, mode, Velocity(1.0, 1.0, true));
	}
	else {
		return DucksActionCommand::stand();
	}
}