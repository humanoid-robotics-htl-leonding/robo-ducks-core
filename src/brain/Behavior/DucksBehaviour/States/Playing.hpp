#pragma once

ActionCommand playing(const DuckDataSet &d)
{
//	auto command = roles(d);
	auto command = ActionCommand::stand();
	if(d.ballState.confident) {
		auto ballPos = d.ballState.position;
		command.combineHead(ActionCommand::Head::lookAt(
			Vector3f(ballPos.x(), ballPos.y(), 0), 4, 4
			));
	}
	return command;
}