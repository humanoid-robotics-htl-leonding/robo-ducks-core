#pragma once

DucksActionCommand roleDefender(const DuckDataSet &d)
{
	auto defenderAction = d.defendingPosition;


	if(defenderAction.valid){
		auto command = walkTo(defenderAction.targetPose, d);
		return command;
	}

	return DucksActionCommand::kneel().invalidate();
}