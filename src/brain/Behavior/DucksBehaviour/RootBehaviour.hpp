//
// Created by obyoxar on 26/11/2019.
//
#pragma once


DucksActionCommand rootBehavior(const DuckDataSet &d)
{
    if(d.cycleInfo.startTime -d.buttonData.lastChestButtonSinglePress <9000 &&(double)d.cycleInfo.startTime>5000 && d.buttonData.lastChestButtonSinglePress >0){
        return DucksActionCommand::keeper(MotionKeeper::MK_TAKE_LEFT);
    }
    return DucksActionCommand::stand();
}