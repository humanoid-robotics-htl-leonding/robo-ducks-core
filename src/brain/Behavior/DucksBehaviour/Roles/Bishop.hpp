#pragma once

DucksActionCommand roleBishop(const DucksDataSet &d)
{
    auto bishopAction = d.bishopPosition;
    if(bishopAction.type == DucksBishopAction::Type::PATROL_AREA)
    {
        return walkTo(bishopAction.targetPose,d,WalkMode::PATH);
    }
    if(bishopAction.type == DucksBishopAction::Type::GO_TO_BALL_POSITION)
    {
        return walkTo(bishopAction.targetPose,d,WalkMode::PATH);
    }
    if(bishopAction.type == DucksBishopAction::Type::DRIBBLE_TO_KICK_LOCATION)
    {
        return walkTo(bishopAction.targetPose, d, WalkMode::DRIBBLE, InWalkKickType::FORWARD);
    }
    if(bishopAction.type == DucksBishopAction::Type::PASS)
    {
        return kick(d,bishopAction.kickTarget);
    }
	return DucksActionCommand::stand().invalidate();
}