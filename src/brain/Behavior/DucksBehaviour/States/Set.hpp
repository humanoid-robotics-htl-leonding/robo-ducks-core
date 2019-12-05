#pragma once

ActionCommand set(const DataSet& d){
  return ActionCommand::dead().combineLeftLED(ActionCommand::EyeLED::colors(1.0, 1.0, 0.0));
}