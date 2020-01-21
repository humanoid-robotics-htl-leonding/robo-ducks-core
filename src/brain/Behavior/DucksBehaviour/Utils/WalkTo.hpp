//
// Created by obyoxar on 10/01/2020.
//
#pragma once

ActionCommand walkTo(const Pose& fieldPose, const DataSet &d) {
	auto localPose = d.robotPosition.fieldToRobot(fieldPose);
	if(localPose.position.norm() > 0.1) { //todo HYSCHTEREESSCHHEEE
		return ActionCommand::walk(localPose, WalkMode::DIRECT, Velocity(1.0, 1.0, true));
	}else{
		return ActionCommand::stand();
	}
}