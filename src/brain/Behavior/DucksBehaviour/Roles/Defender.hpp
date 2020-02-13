#pragma once

ActionCommand roleDefender(const DuckDataSet &d)
{
	auto defenderAction = d.defendingPosition;


	if(defenderAction.valid){

		auto mode = WalkMode::DIRECT;
		if(defenderAction.type == DucksDefendingPosition::DEFEND) {
			mode = WalkMode::DIRECT_WITH_ORIENTATION;
		}


		auto command = walkTo(defenderAction.targetPose, d, mode);
		return command;
	}else{
		Log(LogLevel::WARNING) << "I am defender but no valid DefenderAction was supplied";
	}

	return ActionCommand::kneel();
}