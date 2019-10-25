#pragma once
#include "Behavior/Units.hpp"

ActionCommand rootBehavior(const DataSet& d)
{
  //return ActionCommand::stand().combineAudio(ActionCommand::Audio::test());


  // If the NAO does not have foot contact it overrides the left LED with pink.
  const bool front = d.buttonData.buttons[keys::sensor::SWITCH_HEAD_FRONT];
  const bool middle = d.buttonData.buttons[keys::sensor::SWITCH_HEAD_MIDDLE];
  const bool back = d.buttonData.buttons[keys::sensor::SWITCH_HEAD_REAR];

  if(front){
    return ActionCommand::dead().combineAudio(ActionCommand::Audio::audioC4());
  }else if(middle){
    return ActionCommand::dead().combineAudio(ActionCommand::Audio::audioC3());
  }else if(back){
    return ActionCommand::dead().combineAudio(ActionCommand::Audio::audioC5());
  }else{
    return ActionCommand::dead().combineAudio(ActionCommand::Audio::off());
  }
}
