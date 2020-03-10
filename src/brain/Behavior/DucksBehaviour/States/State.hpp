#pragma once


/**
 * Finished: White (Game has ended)
 * Playing: Green
 * Set: Yellow (Donn)
 * Ready: Blue (Go to your place)
 * Initial: Pink (Chestbutton was pressed. Waiting for Set)
 * Started: Rainbow
 * Penalized: Red
 * @param d
 * @return
 */
DucksActionCommand chooseState(const DucksDataSet &d)
{
	if (d.gameControllerState.penalty != Penalty::NONE) {
		return penalized(d).combineChestLED(DucksActionCommand::ChestLED::red());
	}
	if (
		d.bodyPose.fallen &&
			d.gameControllerState.gameState != GameState::SET &&
			d.gameControllerState.gameState != GameState::INITIAL
		) {
		return DucksActionCommand::standUp()
			.combineLeftFootLED(DucksActionCommand::FootLED::rainbow())
			.combineRightFootLED(DucksActionCommand::FootLED::rainbow());
	}
	switch (d.gameControllerState.gameState) {
		case GameState::INITIAL:
			if (d.gameControllerState.chestButtonWasPressedInInitial) {
				return initial(d).combineChestLED(DucksActionCommand::ChestLED::off());
			}
			else {
				return started(d)
					.combineLeftLED(DucksActionCommand::EyeLED::rainbow())
					.combineRightLED(DucksActionCommand::EyeLED::rainbow())
					.combineChestLED(DucksActionCommand::ChestLED::rainbow())
					.combineLeftFootLED(DucksActionCommand::FootLED::rainbow())
					.combineRightFootLED(DucksActionCommand::FootLED::rainbow())
					.combineLeftEarLED(DucksActionCommand::EarLED::loading())
					.combineRightEarLED(DucksActionCommand::EarLED::loading());
			}
		case GameState::READY: return ready(d).combineChestLED(DucksActionCommand::ChestLED::blue());
		case GameState::SET: return set(d).combineChestLED(DucksActionCommand::ChestLED::yellow());
		case GameState::PLAYING: return playing(d).combineChestLED(DucksActionCommand::ChestLED::green());
		case GameState::FINISHED: return started(d).combineChestLED(DucksActionCommand::ChestLED::off());
		default: return DucksActionCommand::penalized();
	}
}