#pragma once

ActionCommand roleBishop(const DataSet& d){
  return ActionCommand::dead().combineRightLED(ActionCommand::LED::colors(1.0, 1.0, 0.0));
}