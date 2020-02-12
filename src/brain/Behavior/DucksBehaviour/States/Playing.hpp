#pragma once

/**
 *
 * Left Eye:
 *  * Team_ball_model: pink
 *  * Own_camera: red
 *  * searching: blue
 *  * search_walk: light_blue
 *  * i_am_on_it: yellow
 *
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

	//Implement looking at ballSearchPosition
	// -- left LED Red -> We see the ball.
	// -- left LED Pink -> We look at where our team sees the ball.
	if(d.ballSearchPosition.ownSearchPoseValid){
		auto robotPos = d.robotPosition.fieldToRobot(d.ballSearchPosition.searchPosition);
		auto robotSpacePos = Vector3f(robotPos.x(), robotPos.y(), 0.0);
		command.combineHead(ActionCommand::Head::lookAt(robotSpacePos, 1.0, 1.0));
		switch(d.ballSearchPosition.reason){
			case DuckBallSearchPosition::TEAM_BALL_MODEL:
				command.combineLeftLED(ActionCommand::EyeLED::pink());
				break;
			case DuckBallSearchPosition::OWN_CAMERA:
				command.combineLeftLED(ActionCommand::EyeLED::red());
				break;
			case DuckBallSearchPosition::SEARCHING:
				command.combineLeftLED(ActionCommand::EyeLED::blue());
				break;
			case DuckBallSearchPosition::SEARCH_WALK:
				return walkTo(d.ballSearchPosition.pose, d)
				.combineHead(ActionCommand::Head::lookAt(robotSpacePos))
				.combineLeftLED(ActionCommand::EyeLED::lightblue());
			case DuckBallSearchPosition::I_AM_ON_IT:
				return walkTo(d.ballSearchPosition.pose, d).combineLeftLED(ActionCommand::EyeLED::yellow());
			default: break;
		}
	}

	return command;
}