#pragma once

ActionCommand roles(const DataSet& d){
  switch(d.playingRoles.role){
    case PlayingRole::NONE:
      return roleNone(d).combineRightLED(ActionCommand::EyeLED::colors(1.0, 1.0, 1.0));
    case PlayingRole ::BISHOP:
      return roleBishop(d).combineRightLED(ActionCommand::EyeLED::colors(1.0, 1.0, 0.0));
    case PlayingRole ::DEFENDER:
      return roleDefender(d).combineRightLED(ActionCommand::EyeLED::colors(0.0, 1.0, 0.0));
    case PlayingRole ::KEEPER:
    case PlayingRole ::REPLACEMENT_KEEPER:
      return roleKeeper(d).combineRightLED(ActionCommand::EyeLED::colors(0.0, 0.0, 1.0));
    case PlayingRole ::STRIKER:
      return roleStriker(d).combineRightLED(ActionCommand::EyeLED::colors(1.0, 0.0, 0.0));;
    case PlayingRole ::SUPPORT_STRIKER:
      return roleSupportStriker(d).combineRightLED(ActionCommand::EyeLED::colors(1.0, 0.0, 1.0));
      default:
          return ActionCommand::dead();
  }
}