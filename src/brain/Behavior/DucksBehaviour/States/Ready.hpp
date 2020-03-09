#pragma once


DucksActionCommand ready(const DucksDataSet &d)
{
	auto targetPos = d.setPosition.position;

	auto command = walkTo(targetPos, d);


//	auto command = DucksActionCommand::stand().combineLeftArm(DucksActionCommand::Arm::point(Vector3f(1,1,1)));
	if (d.thoughts.handleNewState()) {
		command.combineThoughtCommand(ThoughtCommand::RESET_COMPASS_DIRECTION);
	}
	return command;
}