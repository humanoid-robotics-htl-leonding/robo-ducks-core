#pragma once

ActionCommand chooseState(const DataSet& d){
  std::cout << "" << (int) d.gameControllerState.penalty << std::endl;
  if(d.gameControllerState.penalty != Penalty::NONE){
    return penalized(d);
  }
  switch (d.gameControllerState.gameState){
    case GameState::INITIAL:
      if(d.gameControllerState.chestButtonWasPressedInInitial){
        return initial(d);
      }else{
        return started(d);
      }
    case GameState::READY: return ready(d);
    case GameState::SET: return set(d);
    case GameState::PLAYING: return playing(d);
    case GameState::FINISHED: return started(d);
  }
}