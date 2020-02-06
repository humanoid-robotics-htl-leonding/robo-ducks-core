#pragma once

/**
 * 1. Choose and execute Role
 * 2. a) Look at the ball if we can see it
 * 	  b) Look at where our team thinks the ball is.
 * @param d
 * @return
 */
ActionCommand playing(const DuckDataSet &d)
{
//	auto command = roles(d);
	auto command = ActionCommand::stand();

	//1. === Implement "Look At Ball" (Erik Mayrhofer)
	// -- left LED Red -> We see the ball.
	// -- left LED Pink -> We look at where our team sees the ball.

	// If we see the ball on our own, look at it.
	if (d.ballState.age < 1.0 && d.ballState.confident) {
		auto ballPos = d.ballState.position + d.ballState.velocity*d.cycleInfo.cycleTime;
		command.combineHead(ActionCommand::Head::lookAt(
			Vector3f(ballPos.x(), ballPos.y(), 0), 4, 4
		)).combineLeftLED(ActionCommand::EyeLED::red());
	//If we don't see the ball on our own, look at where our team thinks the ball is. Maybe we can find it there.
	}else if(d.teamBallModel.found && d.teamBallModel.insideField){
		auto ballPos = d.robotPosition.fieldToRobot(d.teamBallModel.position + d.teamBallModel.velocity*d.cycleInfo.cycleTime);
		command.combineHead(ActionCommand::Head::lookAt(
			Vector3f(ballPos.x(), ballPos.y(), 0), 4, 4
		)).combineLeftLED(ActionCommand::EyeLED::pink());
	}
	return command;
}