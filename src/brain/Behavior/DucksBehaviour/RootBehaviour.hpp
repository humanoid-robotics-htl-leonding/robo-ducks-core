//
// Created by obyoxar on 26/11/2019.
//
#pragma once

DucksActionCommand rootBehavior(const DuckDataSet &d)
{
    if(d.cycleInfo.startTime-d.buttonData.lastChestButtonSinglePress <50){
        return DucksActionCommand::keeper(MotionKeeper::MK_TAKE_LEFT);
    }
    return DucksActionCommand::stand();
}