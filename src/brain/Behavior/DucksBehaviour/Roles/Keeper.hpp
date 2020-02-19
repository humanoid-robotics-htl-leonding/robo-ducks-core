#pragma once

DucksActionCommand roleKeeper(const DuckDataSet &d)
{
    if(d.keeperAction.action.valid){
        return walkTo(d.keeperAction.action.pose, d);
    }

    return DucksActionCommand::stand().invalidate();
}