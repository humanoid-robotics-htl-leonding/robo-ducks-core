#pragma once

ActionCommand chooseState(const DataSet& d){
  if(d.gameControllerState.penalty != Penalty::NONE){
    return penalized(d).combineChestLED(ActionCommand::ChestLED::red());
  }
  switch (d.gameControllerState.gameState){
    case GameState::INITIAL:
      if(d.gameControllerState.chestButtonWasPressedInInitial){
        return initial(d).combineChestLED(ActionCommand::ChestLED::pink());
      }else{
        return started(d)
            .combineLeftLED(ActionCommand::EyeLED::rainbow())
            .combineRightLED(ActionCommand::EyeLED::rainbow())
            .combineChestLED(ActionCommand::ChestLED::rainbow())
            .combineLeftFootLED(ActionCommand::FootLED::rainbow())
            .combineRightFootLED(ActionCommand::FootLED::rainbow())
            .combineLeftEarLED(ActionCommand::EarLED::loading())
            .combineRightEarLED(ActionCommand::EarLED::loading());
      }
    case GameState::READY: return ready(d).combineLeftLED(ActionCommand::EyeLED::blue());
    case GameState::SET: return set(d).combineLeftLED(ActionCommand::EyeLED::yellow());
    case GameState::PLAYING: return playing(d).combineChestLED(ActionCommand::ChestLED::green());
    case GameState::FINISHED: return started(d).combineChestLED(ActionCommand::ChestLED::white());
    default: return ActionCommand::dead();
  }
}