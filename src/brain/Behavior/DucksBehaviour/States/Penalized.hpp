#pragma once

ActionCommand penalized(const DataSet& d){
  return ActionCommand::kneel().combineLeftLED(ActionCommand::LED::colors(1.0, 0.0, 0.0));
}