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

DucksActionCommand playing(const DucksDataSet &d)
{
	auto command = roles(d);

	//Implement looking at poi
	// -- left LED Red -> We see the ball.
	// -- left LED Pink -> We look at where our team sees the ball.
//	if(d.poi.ownSearchPoseValid){
//		auto robotPos = d.robotPosition.fieldToRobot(d.poi.searchPosition);
//		auto robotPose = d.robotPosition.fieldToRobot(d.poi.pose);
//		auto robotSpacePos = Vector3f(robotPos.x(), robotPos.y(), 0.0);
//		if(!command.body().valid()){
//			if(
//				d.poi.reason == DuckBallSearchPosition::SEARCH_WALK ||
//				d.poi.reason == DuckBallSearchPosition::I_AM_ON_IT
//				){
//				command = walkTo(d.poi.pose, d);
//			}else if(d.poi.reason == DuckBallSearchPosition::SEARCH_TURN){
//				command = walkTo(d.robotPosition.pose.oriented(d.poi.pose.orientation), d)
//					.combineBodyWalkType(WalkMode::DIRECT_WITH_ORIENTATION);
//			}else{
////				command = DucksActionCommand::stand();
//			}
//		}
//
//		command.combineHead(DucksActionCommand::Head::lookAt(robotSpacePos, 1.0, 1.0));
//
//		switch(d.poi.reason){
//			case DuckBallSearchPosition::TEAM_BALL_MODEL:
//				command.combineLeftLED(DucksActionCommand::EyeLED::pink());
//				break;
//			case DuckBallSearchPosition::OWN_CAMERA:
//				command.combineLeftLED(DucksActionCommand::EyeLED::red());
//				break;
//			case DuckBallSearchPosition::SEARCHING:
//				command.combineLeftLED(DucksActionCommand::EyeLED::blue());
//				break;
//			case DuckBallSearchPosition::SEARCH_WALK:
//				command.combineLeftLED(DucksActionCommand::EyeLED::lightblue());
//				break;
//			case DuckBallSearchPosition::SEARCH_TURN:
//				command.combineLeftLED(DucksActionCommand::EyeLED::blue());
//				break;
//			case DuckBallSearchPosition::I_AM_ON_IT:
//				command.combineLeftLED(DucksActionCommand::EyeLED::yellow());
//				break;
//			case DuckBallSearchPosition::LOOK_AROUND:
//				command.combineLeftEarLED(DucksActionCommand::EyeLED::green());
//			default: break;
//		}
//
//	}

	return command;
}