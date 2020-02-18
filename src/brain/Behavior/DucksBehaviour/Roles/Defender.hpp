#pragma once

DucksActionCommand roleDefender(const DucksDataSet &d)
{
	auto defenderAction = d.defendingPosition;


	if(defenderAction.valid){
		auto command = walkTo(defenderAction.targetPose, d);
		return command;
	}

	return DucksActionCommand::kneel().invalidate();
}