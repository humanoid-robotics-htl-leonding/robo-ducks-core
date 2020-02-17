#pragma once

ActionCommand kickBall(const DuckDataSet &d, const StrikerAction &sa)
{
	if (sa.kickable != BallUtils::Kickable::NOT) {
		return ActionCommand::kick(d.ballState.position,
								   sa.target);  // TODO Wrong Kick Type (use SA)
	}
	else {
		return walkTo(sa.kickPose, d);
	}
}

ActionCommand roleStriker(const DuckDataSet &d)
{
//    d.robotPosition.robotToField()
//    ActionCommand cmd = ActionCommand::walk(d.robotPosition.robotToField(Pose(0, 0, 45*TO_RAD)));

	auto sa = d.strikerAction;

	if (sa.valid) {
		switch (sa.type) {
			case StrikerAction::KICK_INTO_GOAL: return kickBall(d, sa);
			default: Log(LogLevel::ERROR) << "StrikerActionType: " << sa.type << "is not implemented yet.";
		}
	}
	else {
		Log(LogLevel::WARNING) << "Invalid Striker Action";
	}

	return ActionCommand::stand();
}