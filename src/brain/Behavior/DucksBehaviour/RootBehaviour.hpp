//
// Created by obyoxar on 26/11/2019.
//
#pragma once

ActionCommand rootBehavior(const DuckDataSet &d)
{
    if(d.cycleInfo.startTime-d.buttonData.lastChestButtonSinglePress<50){
        Vector2f source =Vector2f(0.2,0.06);
        Vector2f target =Vector2f(1.7,0.9);
        return ActionCommand::kick(source,target);
    }
    return ActionCommand::stand();
}