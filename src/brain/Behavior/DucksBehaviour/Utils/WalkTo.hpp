//
// Created by obyoxar on 10/01/2020.
//
#pragma once

ActionCommand walkTo(const Pose &fieldPose, const DuckDataSet &d)
{
	auto localPose = d.robotPosition.fieldToRobot(fieldPose);
	if (localPose.position.norm() > 0.1 || std::abs(localPose.orientation) > 0.05) { //todo HYSCHTEREESSCHHEEE
		return ActionCommand::walk(localPose, WalkMode::DIRECT_WITH_ORIENTATION, Velocity(1.0, 1.0, true));
	}
	else {
		return ActionCommand::stand();
	}
}