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
	auto command = roles(d);

	//Implement looking at ballSearchPosition
	// -- left LED Red -> We see the ball.
	// -- left LED Pink -> We look at where our team sees the ball.
	if(d.ballSearchPosition.ownSearchPoseValid){
		auto robotPos = d.robotPosition.fieldToRobot(d.ballSearchPosition.searchPosition);
		auto robotPose = d.robotPosition.fieldToRobot(d.ballSearchPosition.pose);
		auto robotSpacePos = Vector3f(robotPos.x(), robotPos.y(), 0.0);
		if(d.ballSearchPosition.desperate){
			if(d.ballSearchPosition.reason == DuckBallSearchPosition::SEARCH_WALK){
				command = walkTo(d.ballSearchPosition.pose, d)
					.combineHead(ActionCommand::Head::lookAt(robotSpacePos))
					.combineLeftLED(ActionCommand::EyeLED::lightblue());
			}else if(d.ballSearchPosition.reason == DuckBallSearchPosition::SEARCH_TURN){
				command
					.combineBodyWalkType(WalkMode::DIRECT_WITH_ORIENTATION)
					.combineBodyWalkTargetOrientation(robotPose.orientation)
					.combineHead(ActionCommand::Head::lookAt(robotSpacePos))
					.combineLeftLED(ActionCommand::EyeLED::blue());
			}else if(d.ballSearchPosition.reason == DuckBallSearchPosition::I_AM_ON_IT){
				command = walkTo(d.ballSearchPosition.pose, d).combineLeftLED(ActionCommand::EyeLED::yellow());
			}
		}

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
				command.combineLeftLED(ActionCommand::EyeLED::lightblue());
				break;
			case DuckBallSearchPosition::SEARCH_TURN:
				command.combineLeftLED(ActionCommand::EyeLED::blue());
				break;
			case DuckBallSearchPosition::I_AM_ON_IT:
				command.combineLeftLED(ActionCommand::EyeLED::yellow());
				break;
			default: break;
		}

	}

	return command;
}