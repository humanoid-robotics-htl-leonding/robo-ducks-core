//
// Created by obyoxar on 03/12/2019.
//

#include "Thoughts.hpp"

Thoughts::Thoughts()
  : stateSince(0)
  , isStateNew(false)
  , gameState(GameState::INITIAL)
{

}

void Thoughts::update(DuckDataSet& d) {
  if (gameState != d.gameControllerState.gameState) {
    stateSince = d.cycleInfo.startTime;
    isStateNew = true;
    gameState = d.gameControllerState.gameState;
  }
}

bool Thoughts::handleNewState() {
  if(isStateNew){
    isStateNew = false;
    return true;
  }
  return false;
}
