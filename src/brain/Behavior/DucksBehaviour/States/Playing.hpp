#pragma once

ActionCommand playing(const DuckDataSet &d)
{
//	auto command = roles(d);
	auto command = ActionCommand::stand();
	if (d.ballState.age < 1.0 && d.ballState.confident) {
		auto ballPos = d.ballState.position + d.ballState.velocity*d.cycleInfo.cycleTime;
		command.combineHead(ActionCommand::Head::lookAt(
			Vector3f(ballPos.x(), ballPos.y(), 0), 4, 4
		)).combineLeftLED(ActionCommand::EyeLED::red());
	}else if(d.teamBallModel.found && d.teamBallModel.insideField){
		auto ballPos = d.robotPosition.fieldToRobot(d.teamBallModel.position + d.teamBallModel.velocity*d.cycleInfo.cycleTime);
		command.combineHead(ActionCommand::Head::lookAt(
			Vector3f(ballPos.x(), ballPos.y(), 0), 4, 4
		)).combineLeftLED(ActionCommand::EyeLED::pink());
	}
	return command;
}