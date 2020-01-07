#pragma once

ActionCommand ready(const DataSet &d)
{
//	auto targetPos = d.setPosition.position;
//	auto command = ActionCommand::walk(Pose(10.0, 10.0, 9), WalkMode::DIRECT, Velocity());
	auto command = ActionCommand::stand();
	if (d.thoughts.handleNewState()) {
		command.combineThoughtCommand(ThoughtCommand::RESET_COMPASS_DIRECTION);
	}
	return command;
}