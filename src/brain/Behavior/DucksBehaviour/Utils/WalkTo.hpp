//
// Created by obyoxar on 10/01/2020.
//
#pragma once

DucksActionCommand walkTo(const Pose &fieldPose, const DucksDataSet &d)
{
	auto localPose = d.robotPosition.fieldToRobot(fieldPose);
	if (localPose.position.norm() > 0.05 || std::abs(localPose.orientation) > 5*TO_RAD) { //todo HYSCHTEREESSCHHEEE
		return DucksActionCommand::walk(localPose, WalkMode::DIRECT, Velocity(1.0, 1.0, true));
	}
	else {
		return DucksActionCommand::stand();
	}
}