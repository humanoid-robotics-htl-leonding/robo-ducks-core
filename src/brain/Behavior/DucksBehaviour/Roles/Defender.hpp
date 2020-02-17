#pragma once

DucksActionCommand roleDefender(const DuckDataSet &d)
{
	auto defenderAction = d.defendingPosition;


	if(defenderAction.valid){

		auto mode = WalkMode::DIRECT;
		if(defenderAction.type == DucksDefendingPosition::DEFEND) {
			mode = WalkMode::DIRECT_WITH_ORIENTATION;
		}


		auto command = walkTo(defenderAction.targetPose, d, mode);
		return command;
	}

	return DucksActionCommand::kneel().invalidate();
}