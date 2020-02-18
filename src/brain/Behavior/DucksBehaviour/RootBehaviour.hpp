//
// Created by obyoxar on 26/11/2019.
//
#pragma once

DucksActionCommand rootBehavior(const DuckDataSet &d)
{
    //TODO check all motion files if they still work
    //TODO jumpingCatch extend second arm
    //TODO stationaryCatch find Foot Balance
    //TODO buttcatch check if penetrable
    //TODO flip fixed jumping and stationaryCatch
    if(d.cycleInfo.startTime-d.buttonData.lastChestButtonSinglePress <50){
        return DucksActionCommand::keeper(MotionKeeper::MK_TAKE_LEFT);
    }
    return DucksActionCommand::stand();
}