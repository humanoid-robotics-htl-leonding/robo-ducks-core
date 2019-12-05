#pragma once

ActionCommand roleKeeper(const DataSet& d){
  return ActionCommand::dead().combineRightLED(ActionCommand::LED::colors(0.0, 0.0, 1.0));
}