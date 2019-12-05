#pragma once

ActionCommand ready(const DataSet& d){
  auto command = ActionCommand::stand();
  if(d.thoughts.handleNewState()){
    command.combineThoughtCommand(ThoughtCommand::RESET_COMPASS_DIRECTION);
  }
  return command;
}