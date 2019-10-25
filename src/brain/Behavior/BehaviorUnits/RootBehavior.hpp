#pragma once
#include "Behavior/Units.hpp"

ActionCommand rootBehavior(const DataSet& d)
{
  const bool high = !d.bodyPose.footContact;

  if (d.gameControllerState.penalty == Penalty::NONE)
  {
    return high ? notPenalized(d).combineLeftLED(ActionCommand::LED::pink()) : notPenalized(d);
  }
  else
  {
    return high ? ActionCommand::penalized().combineLeftLED(ActionCommand::LED::pink()) : ActionCommand::penalized();
  }
}
