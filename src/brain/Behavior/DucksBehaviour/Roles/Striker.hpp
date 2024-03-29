#pragma once

DucksActionCommand kickBall(const DucksDataSet &d, const DucksStrikerAction &sa)
{
    return DucksActionCommand::kick(d.ballState.position, //ballState.position is not good here, a new StrikerAction.kickSource or something like that would be useful, rotated like StrikerAction.target
                               sa.target);  // TODO Wrong Kick Type (use SA)
}

DucksActionCommand roleStriker(const DucksDataSet &d)
{
    if (d.strikerAction.valid) {
        switch (d.strikerAction.action) {
            case DucksStrikerAction::Action::WALK_TO_POS:
                return walkTo(d.strikerAction.kickPose, d);
            case DucksStrikerAction::Action::KICK_INTO_GOAL:
                return kickBall(d, d.strikerAction);
            case DucksStrikerAction::Action::WAITING_FOR_BALL:
                return walkTo(d.strikerAction.kickPose, d /*,WalkMode::DRIBBLE*/);
            case DucksStrikerAction::Action::DRIBBLE_TO_POS:
                return walkTo(d.strikerAction.kickPose, d, WalkMode::DRIBBLE);
            default:
                Log(LogLevel::WARNING) << "Invalid Striker Action";
        }
    }
//    d.robotPosition.robotToField()
//    DucksActionCommand cmd = DucksActionCommand::walk(d.robotPosition.robotToField(Pose(0, 0, 45*TO_RAD)));

//	auto sa = d.strikerAcction;
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