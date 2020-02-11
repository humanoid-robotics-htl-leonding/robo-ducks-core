//
// Created by obyoxar on 26/11/2019.
//
#pragma once

ActionCommand rootBehavior(const DuckDataSet &d)
{
	if(d.cycleInfo.startTime-d.buttonData.lastChestButtonSinglePress<50){
          Vector2f source =Vector2f(0.2,0.06);
          Vector2f target =Vector2f(3,0.06);
          return ActionCommand::kick(source,target,KickType::FORWARD);
    }
	return ActionCommand::stand();
}