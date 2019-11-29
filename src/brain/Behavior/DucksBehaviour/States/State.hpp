#pragma once

ActionCommand chooseState(const DataSet& d){
  if(d.gameControllerState.gameState == GameState::INITIAL){
    if(d.gameControllerState.chestButtonWasPressedInInitial){
      return initial(d);
    }else{
      return started(d);
    }
  }else{
    return ActionCommand::dead();
  }
}