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
DucksActionCommand roles(const DucksDataSet &d)
{
	switch (d.playingRoles.role) {
	    case PlayingRole::NONE:return roleNone(d).combineRightLED(DucksActionCommand::EyeLED::white());
		case PlayingRole::BISHOP:return roleBishop(d).combineRightLED(DucksActionCommand::EyeLED::yellow());
		case PlayingRole::DEFENDER:return roleDefender(d).combineRightLED(DucksActionCommand::EyeLED::green());
		case PlayingRole::KEEPER://case continues in the replacement_KeeperCase
		case PlayingRole::REPLACEMENT_KEEPER:
			return roleKeeper(d).combineRightLED(DucksActionCommand::EyeLED::blue());
		case PlayingRole::STRIKER:return roleStriker(d).combineRightLED(DucksActionCommand::EyeLED::red());;
		case PlayingRole::SUPPORT_STRIKER:
			return roleSupportStriker(d).combineRightLED(DucksActionCommand::EyeLED::violet());
		default: return DucksActionCommand::stand().invalidate();
	}
}