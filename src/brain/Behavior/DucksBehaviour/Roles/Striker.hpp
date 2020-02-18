#pragma once

DucksActionCommand kickBall(const DuckDataSet &d, const DucksStrikerAction &sa)
{
    return DucksActionCommand::kick(d.ballState.position,
                               sa.target);  // TODO Wrong Kick Type (use SA)
}

DucksActionCommand roleStriker(const DuckDataSet &d)
{
    if (d.strikerAction.valid) {
        switch (d.strikerAction.action) {
            case DucksStrikerAction::Action::WALK_TO_BALL:
                return walkTo(d.strikerAction.kickPose, d);
            case DucksStrikerAction::Action::KICK_INTO_GOAL:
                return kickBall(d, d.strikerAction);
            default:
                Log(LogLevel::WARNING) << "Invalid Striker Action";
        }
    }
//    d.robotPosition.robotToField()
//    DucksActionCommand cmd = DucksActionCommand::walk(d.robotPosition.robotToField(Pose(0, 0, 45*TO_RAD)));

//	auto sa = d.strikerAction;
//
//	if (sa.valid) {
//		switch (sa.type) {
//			case StrikerAction::KICK_INTO_GOAL: return kickBall(d, sa);
//			default: Log(LogLevel::ERROR) << "StrikerActionType: " << sa.type << "is not implemented yet.";
//		}
//	}
//	else {
//		Log(LogLevel::WARNING) << "Invalid Striker Action";
//	}

	return DucksActionCommand::stand().invalidate();
}