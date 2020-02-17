#pragma once

DucksActionCommand initial(const DuckDataSet &d)
{
	auto command = DucksActionCommand::stand();

	if (d.thoughts.handleNewState()) {
		command.combineThoughtCommand(ThoughtCommand::RESET_COMPASS_DIRECTION);
	}
	return command;
}