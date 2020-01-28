#pragma once



ActionCommand ready(const DuckDataSet &d)
{
	auto targetPos = d.setPosition.position;


	auto command = walkTo(targetPos, d);


//	auto command = ActionCommand::stand().combineLeftArm(ActionCommand::Arm::point(Vector3f(1,1,1)));
	if (d.thoughts.handleNewState()) {
		command.combineThoughtCommand(ThoughtCommand::RESET_COMPASS_DIRECTION);
	}
	return command;
}