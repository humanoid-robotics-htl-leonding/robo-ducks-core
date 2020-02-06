#pragma once

/**
 * None: White
 * Bishop: Yellow
 * Defender: Green
 * Keeper / Replacement Keeper: Blue
 * Striker: Red
 * Support Striker: Violet
 * @param d
 * @return
 */
ActionCommand roles(const DuckDataSet &d)
{
	switch (d.playingRoles.role) {
	    case PlayingRole::NONE:return roleNone(d).combineRightLED(ActionCommand::EyeLED::white());
		case PlayingRole::BISHOP:return roleBishop(d).combineRightLED(ActionCommand::EyeLED::yellow());
		case PlayingRole::DEFENDER:return roleDefender(d).combineRightLED(ActionCommand::EyeLED::green());
		case PlayingRole::KEEPER://case continues in the replacement_KeeperCase
		case PlayingRole::REPLACEMENT_KEEPER:
			return roleKeeper(d).combineRightLED(ActionCommand::EyeLED::blue());
		case PlayingRole::STRIKER:return roleStriker(d).combineRightLED(ActionCommand::EyeLED::red());;
		case PlayingRole::SUPPORT_STRIKER:
			return roleSupportStriker(d).combineRightLED(ActionCommand::EyeLED::violet());
		default: return ActionCommand::kneel();
	}
}