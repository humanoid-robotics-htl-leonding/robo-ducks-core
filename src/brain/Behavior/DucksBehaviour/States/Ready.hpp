#pragma once

ActionCommand ready(const DataSet& d){
  return ActionCommand::dead().combineLeftLED(ActionCommand::LED::colors(0.0, 0.0, 1.0));
}