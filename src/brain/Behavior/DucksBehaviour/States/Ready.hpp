#pragma once

ActionCommand ready(const DataSet& d){
  auto command = ActionCommand::dead().combineLeftLED(ActionCommand::EyeLED::colors(0.0, 0.0, 1.0));
  if(d.thoughts.handleNewState()){
    command.combineThoughtCommand(ThoughtCommand::RESET_COMPASS_DIRECTION);
  }
  return command;
}