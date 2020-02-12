#pragma once

#include <vector>

#include "Framework/DataType.hpp"


/// Definition of playing roles. If one changes this, one also has to change the BHULKs Role enum
/// and the BHULKsHelper
enum class PlayingRole : uint8_t
{
  NONE = 0,
  KEEPER = 1,
  DEFENDER = 2,
  SUPPORT_STRIKER = 3,
  STRIKER = 4,
  BISHOP = 5,
  REPLACEMENT_KEEPER = 6,

  NUM_ROLES
};

inline void operator>>(const Uni::Value& in, PlayingRole& out)
{
  int readValue;
  in >> readValue;
  out = static_cast<PlayingRole>(readValue);
}

inline void operator<<(Uni::Value& out, const PlayingRole& in)
{
  out << static_cast<uint8_t>(in);
}

class PlayingRoles : public DataType<PlayingRoles>
{
public:
  /// the name of this DataType
  DataTypeName name = "PlayingRoles";
  /// the role the robot is assigned to
  PlayingRole role = PlayingRole::NONE;
  /// the roles of all players (playerNumber-1 → role)
  std::vector<PlayingRole> playerRoles;
  /**
   * @brief reset sets the ball to a defined state
   */
  void reset()
  {
    role = PlayingRole::NONE;
    playerRoles.clear();
  }

  virtual void toValue(Uni::Value& value) const
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["role"] << role;
    value["playerRoles"] << playerRoles;
  }

  virtual void fromValue(const Uni::Value& value)
  {
    value["role"] >> role;
    value["playerRoles"] >> playerRoles;
  }
};
