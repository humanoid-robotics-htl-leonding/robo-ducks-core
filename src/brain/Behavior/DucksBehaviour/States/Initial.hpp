#pragma once

ActionCommand initial(const DataSet& d){
  auto command = ActionCommand::stand().combineLeftLED(ActionCommand::LED::colors(1.0, 0.0, 1.0));

  if(d.thoughts.handleNewState()){
    command.combineThoughtCommand(ThoughtCommand::RESET_COMPASS_DIRECTION);
  }
  return command;
}