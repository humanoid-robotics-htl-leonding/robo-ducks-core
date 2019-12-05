#pragma once

ActionCommand roles(const DataSet& d){
  switch(d.playingRoles.role){
    case PlayingRole::NONE:
      return roleNone(d);
    case PlayingRole ::BISHOP:
      return roleBishop(d);
    case PlayingRole ::DEFENDER:
      return roleDefender(d);
    case PlayingRole ::KEEPER:
    case PlayingRole ::REPLACEMENT_KEEPER:
      return roleKeeper(d);
    case PlayingRole ::STRIKER:
      return roleStriker(d);
    case PlayingRole ::SUPPORT_STRIKER:
      return roleSupportStriker(d);
  }
}