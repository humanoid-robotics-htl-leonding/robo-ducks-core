#pragma once

ActionCommand roleDefender(const DataSet& d){
  return ActionCommand::dead().combineRightLED(ActionCommand::LED::colors(0.0, 1.0, 0.0));
}