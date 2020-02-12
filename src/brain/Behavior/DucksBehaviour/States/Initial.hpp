#pragma once

ActionCommand initial(const DuckDataSet &d)
{
	auto command = ActionCommand::stand();

	if (d.thoughts.handleNewState()) {
		command.combineThoughtCommand(ThoughtCommand::RESET_COMPASS_DIRECTION);
	}
	return command;
}