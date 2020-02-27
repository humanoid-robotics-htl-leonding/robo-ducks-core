//
// Created by obyoxar on 26/11/2019.
//
#pragma once


DucksActionCommand rootBehavior(const DuckDataSet &d)
{
    //TODO check all motion files if they still work
    //TODO stationaryCatch find Foot Balance
    //TODO flip fixed stationaryCatch
    if(d.cycleInfo.startTime -d.buttonData.lastChestButtonSinglePress <3000 &&(double)d.cycleInfo.startTime>5000){
        return DucksActionCommand::keeper(MotionKeeper::MK_JUMP_RIGHT);
    }
    return DucksActionCommand::stand();
}