#pragma once

DucksActionCommand roleStriker(const DucksDataSet &d)
{
    if (d.strikerAction.valid) {
        switch (d.strikerAction.action) {
            case DucksStrikerAction::Action::WALK_TO_POS:
                return walkTo(d.strikerAction.walkTarget, d);
            case DucksStrikerAction::Action::KICK_TO_POS:
                //ballState.position is not good here, a new StrikerAction.kickSource or something like that would be useful, rotated like StrikerAction.target
                // TODO Wrong Kick Type (use SA)
                return kick(d, d.strikerAction.target);
            case DucksStrikerAction::Action::WAITING_FOR_BALL:
                return walkTo(d.strikerAction.walkTarget, d);
            case DucksStrikerAction::Action::DRIBBLE_TO_POS:
                return walkTo(d.strikerAction.walkTarget, d, WalkMode::DRIBBLE, InWalkKickType::FORWARD);
            default:
                Log(LogLevel::WARNING) << "Invalid Striker Action";
        }
    }

	return DucksActionCommand::stand().invalidate();
}