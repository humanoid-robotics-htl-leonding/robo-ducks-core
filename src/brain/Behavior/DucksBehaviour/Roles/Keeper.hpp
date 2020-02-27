#pragma once

DucksActionCommand roleKeeper(const DucksDataSet &d)
{
    if(d.keeperAction.action.valid){
        return walkTo(d.keeperAction.action.pose, d);
    }

    return DucksActionCommand::stand().invalidate();
}