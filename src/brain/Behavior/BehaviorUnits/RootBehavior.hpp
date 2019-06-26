#pragma once
#include "Behavior/Units.hpp"

ActionCommand rootBehavior(const DataSet& d)
{
  //return ActionCommand::stand().combineAudio(ActionCommand::Audio::test());


  // If the NAO does not have foot contact it overrides the left LED with pink.
  const bool high = !d.bodyPose.footContact;

  if(high){
    return ActionCommand::stand().combineAudio(ActionCommand::Audio::test());
  }else{
    return ActionCommand::stand().combineAudio(ActionCommand::Audio::test2());
  }

  if (d.gameControllerState.penalty == Penalty::NONE)
  {
    return high ? notPenalized(d).combineLeftLED(ActionCommand::LED::pink()) : notPenalized(d);
  }
  else
  {
    return high ? ActionCommand::penalized().combineLeftLED(ActionCommand::LED::pink()) : ActionCommand::penalized();
  }
}
