#pragma once

ActionCommand started(const DataSet& d){
  return ActionCommand::dead().combineLeftLED(ActionCommand::EyeLED::colors(0.0, 1.0, 1.0));
}