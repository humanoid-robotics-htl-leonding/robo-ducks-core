#pragma once

ActionCommand playing(const DataSet& d){
  return ActionCommand::dead().combineLeftLED(ActionCommand::LED::colors(1.0, 0.0, 0.0));
}