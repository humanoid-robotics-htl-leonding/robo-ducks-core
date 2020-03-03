#pragma once

DucksActionCommand roleDefender(const DucksDataSet &d)
{
	auto defenderAction = d.defendingPosition;


	if(defenderAction.valid){

		WalkMode mode;
		switch (defenderAction.type) {
			case DucksDefenderAction::Type::DEFEND:
				mode = WalkMode::DIRECT_WITH_ORIENTATION;
				return walkTo(defenderAction.targetPose, d, mode);
			case DucksDefenderAction::Type::WALK:
				mode = WalkMode::PATH;
				return walkTo(defenderAction.targetPose, d, mode);
			case DucksDefenderAction::Type::WALK_WITH_ORIENTATION:
				mode = WalkMode::PATH_WITH_ORIENTATION;
				return walkTo(defenderAction.targetPose, d, mode);
			case DucksDefenderAction::Type::WALKDIRECT:
				mode = WalkMode::DIRECT;
				return walkTo(defenderAction.targetPose, d, mode);
			case DucksDefenderAction::Type::WALKDIRECT_WITH_ORIENTATION:
				mode = WalkMode::DIRECT_WITH_ORIENTATION;
				return walkTo(defenderAction.targetPose, d, mode);
			case DucksDefenderAction::Type::KICK:
				return DucksActionCommand::kick(d.ballState.position, Vector2f(1, 0));
			default:
				return DucksActionCommand::stand().invalidate();
		}

	}else{
		Log(LogLevel::WARNING) << "I am defender but no valid DefenderAction was supplied";
	}

	return DucksActionCommand::kneel().invalidate();
}