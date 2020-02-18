#pragma once

DucksActionCommand kickBall(const DucksDataSet &d, const StrikerAction &sa)
{
	if (sa.kickable != BallUtils::Kickable::NOT) {
		return DucksActionCommand::kick(d.ballState.position,
								   sa.target);  // TODO Wrong Kick Type (use SA)
	}
	else {
		return walkTo(sa.kickPose, d);
	}
}

DucksActionCommand roleStriker(const DucksDataSet &d)
{
//    d.robotPosition.robotToField()
//    DucksActionCommand cmd = DucksActionCommand::walk(d.robotPosition.robotToField(Pose(0, 0, 45*TO_RAD)));

	auto sa = d.strikerAction;

	if (sa.valid) {
		switch (sa.type) {
			case StrikerAction::KICK_INTO_GOAL: return kickBall(d, sa);
			default: Log(LogLevel::ERROR) << "StrikerActionType: " << sa.type << "is not implemented yet.";
		}
	}

	return DucksActionCommand::stand().invalidate();
}