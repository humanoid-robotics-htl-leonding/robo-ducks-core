#pragma once

ActionCommand roleSupportStriker(const DataSet& d){
  return ActionCommand::dead().combineRightLED(ActionCommand::EyeLED::colors(1.0, 0.0, 1.0));
}