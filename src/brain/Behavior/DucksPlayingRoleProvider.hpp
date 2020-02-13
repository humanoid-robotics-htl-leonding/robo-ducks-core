#pragma once

#include <string>

#include "Data/BodyPose.hpp"
#include "Data/CycleInfo.hpp"
#include "Data/FieldDimensions.hpp"
#include "Data/GameControllerState.hpp"
#include "Data/PlayerConfiguration.hpp"
#include "Data/PlayingRoles.hpp"
#include "Data/RobotPosition.hpp"
#include "Data/TeamBallModel.hpp"
#include "Data/TeamPlayers.hpp"
#include "Data/TimeToReachBall.hpp"
#include "Data/WorldState.hpp"
#include "Framework/Module.hpp"


class Brain;

class DucksPlayingRoleProvider : public Module<DucksPlayingRoleProvider, Brain>
{
public:
  /// the name of this module
  ModuleName name = "DucksPlayingRoleProvider";
  DucksPlayingRoleProvider(const ModuleManagerInterface& manager);
  void cycle();

private:
  struct Player
  {
    Player(const unsigned int playerNumber, const float x)
      : playerNumber(playerNumber)
      , x(x)
    {
    }
    unsigned int playerNumber;
    float x;
  };

  const Parameter<bool> useTeamRole_;
  const Parameter<bool> assignBishop_;
  const Parameter<bool> assignBishopWithLessThanFourFieldPlayers_;
  const Parameter<bool> playerOneCanBecomeStriker_;
  const Parameter<bool> allowReplacementKeeper_;
  const Parameter<float> playerOneDistanceThreshold_;
  const Parameter<float> keeperTimeToReachBallPenalty_;
  const Parameter<std::string> forceRole_;
  const Dependency<FieldDimensions> fieldDimensions_;
  const Dependency<PlayerConfiguration> playerConfiguration_;
  const Dependency<RobotPosition> robotPosition_;
  const Dependency<TeamPlayers> teamPlayers_;
  const Dependency<TeamBallModel> teamBallModel_;
  const Dependency<GameControllerState> gameControllerState_;
  const Dependency<BodyPose> bodyPose_;
  const Dependency<CycleInfo> cycleInfo_;
  const Dependency<TimeToReachBall> timeToReachBall_;
  const Dependency<WorldState> worldState_;

  Production<PlayingRoles> playingRoles_;

  std::vector<PlayingRole> lastAssignment_;

  /// whether the robot with player number one is far away from own goal
  bool playerOneWasFarAway_;

  /**
   * @brief assign the striker role
   */
  void assignStriker();

  /**
   * @brief assign the keeper role
   */
  void assignKeeper();

  /**
   * @brief assign the replacement keeper role
   */
  void assignReplacementKeeper();

  /**
   * @brief
   * @param position
   * @param playerNumber
   * @return
   */
  float getDistanceToGoal(const Vector2f position, const unsigned int playerNumber) const;

  /**
   * @brief whether robot with player number one is far away form our own goal
   * @return true if robot with player number one is far away from our own goal
   */
  bool playerOneIsFarAway();

  /**
   * @brief Assign remaining players to other roles
   */
  void assignRemainingPlayerRoles();

  /**
   * @brief set role of player with given number
   * @param playerNumber the number of the players of whom the role is to be set
   * @param role the role to be set
   */
  void updateRole(const unsigned int playerNumber, const PlayingRole role);

  /**
   * @brief convert role string to role enum
   * @param configRole the role string to be converted
   */
  PlayingRole toRole(const std::string& configRole) const;

  /**
   * @brief get actual time to reach ball for given player number
   * @param player number the number of the player of whom the actual time to reach ball is to be
   * determined
   * @param timeToReachBall the time to reach ball
   * @param timeToReachBallStriker the time to reach ball if player were striker
   */
  float actualTimeToReachBall(const unsigned int playerNumber, const float timeToReachBall,
                              const float timeToReachBallStriker) const;
  /**
   * @brief get last role for given player number
   * @param playerNumber the number of the player of whom the role is to be determined
   */
  PlayingRole lastRoleOf(const unsigned int playerNumber) const;
};
