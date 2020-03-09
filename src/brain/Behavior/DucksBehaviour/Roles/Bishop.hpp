#pragma once

DucksActionCommand roleBishop(const DucksDataSet &d)
{
    auto bishopAction = d.bishopPosition;
    if(bishopAction.type == DucksBishopAction::Type::PATROL_AREA)
    {
        return walkTo(bishopAction.targetPose,d,WalkMode::PATH);
    }
	return DucksActionCommand::stand().invalidate();
}