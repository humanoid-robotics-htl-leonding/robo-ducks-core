#pragma once

DucksActionCommand roleKeeper(const DuckDataSet &d)
{
    if(d.keeperAction.action.valid){
    	if(d.keeperAction.action.type == KeeperAction::Type::KICK_AWAY){
			return kick(d, Vector2f(d.fieldDimensions.fieldLength/2.f, 0));
    	}else{
			return walkTo(d.keeperAction.action.pose, d, WalkMode::PATH);
		}
    }

    return DucksActionCommand::stand().invalidate();
}