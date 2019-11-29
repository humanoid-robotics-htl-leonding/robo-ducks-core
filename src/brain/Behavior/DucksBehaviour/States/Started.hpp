#pragma once

ActionCommand started(const DataSet& d){
  return ActionCommand::dead().combineLeftLED(ActionCommand::LED::colors(1.0, 1.0, 1.0));
}