#pragma once

ActionCommand initial(const DataSet& d){
  return ActionCommand::stand().combineLeftLED(ActionCommand::LED::colors(1.0, 0.0, 1.0));
}