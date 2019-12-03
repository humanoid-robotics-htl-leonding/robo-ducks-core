//
// Created by obyoxar on 03/12/2019.
//

#include "Thoughts.hpp"

Thoughts::Thoughts()
  : stateSince(0)
  , gameState(GameState::INITIAL)
{

}

void Thoughts::update(DataSet& d) {
  if (gameState != d.gameControllerState.gameState) {
    stateSince = d.cycleInfo.startTime;
    gameState = d.gameControllerState.gameState;
  }
}

//bool Thoughts::handleNewState() {
//  if(isStateNew){
//    isStateNew = false;
//    return true;
//  }
//  return false;
//}
