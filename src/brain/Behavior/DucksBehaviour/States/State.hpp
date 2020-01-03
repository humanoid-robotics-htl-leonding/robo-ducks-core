#pragma once


/**
 * Finished: White (Game has ended)
 * Playing: Green
 * Set: Yellow (Donn)
 * Ready: Blue (Go to your place)
 * Initial: Pink (Chestbutton was pressed. Waiting for Set)
 * Started: Rainbow
 * Penalized: Red
 * @param d
 * @return
 */
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
    case GameState::READY: return ready(d).combineChestLED(ActionCommand::ChestLED::blue());
    case GameState::SET: return set(d).combineChestLED(ActionCommand::ChestLED::yellow());
    case GameState::PLAYING: return playing(d).combineChestLED(ActionCommand::ChestLED::green());
    case GameState::FINISHED: return started(d).combineChestLED(ActionCommand::ChestLED::white());
    default: return ActionCommand::dead();
  }
}