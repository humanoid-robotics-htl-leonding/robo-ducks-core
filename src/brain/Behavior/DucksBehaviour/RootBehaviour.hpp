//
// Created by obyoxar on 26/11/2019.
//
#pragma once

ActionCommand rootBehavior(const DataSet& d) {
  if(d.gameControllerState.gameState == GameState::INITIAL){
    return ActionCommand::dead().combineLeftLED(ActionCommand::LED::rainbow()).combineRightLED(ActionCommand::LED::rainbow());
  }else{
    return ActionCommand::dead();
  }
}