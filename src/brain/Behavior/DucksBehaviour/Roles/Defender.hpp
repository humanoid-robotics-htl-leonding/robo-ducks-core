#pragma once

ActionCommand roleDefender(const DuckDataSet &d)
{
	auto defenderAction = d.defendingPosition;


	if(defenderAction.valid){
		auto command = walkTo(defenderAction.targetPose, d);
		return command;
	}else{
		Log(LogLevel::WARNING) << "I am defender but no valid DefenderAction was supplied";
	}

	return ActionCommand::kneel();
}