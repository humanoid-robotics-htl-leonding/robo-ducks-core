#pragma once

#include <Data/DucksStrikerAction.hpp>
#include "Data/BodyPose.hpp"
#include "Data/GameControllerState.hpp"
#include "Data/RobotPosition.hpp"
#include "Data/StrikerAction.hpp"
#include "Data/TeamBallModel.hpp"
#include "Data/TimeToReachBall.hpp"
#include "Data/WalkingEngineWalkOutput.hpp"
#include "Framework/Module.hpp"


class Brain;

class DucksTimeToReachBallProvider : public Module<DucksTimeToReachBallProvider, Brain>
{
public:
  /// the name of this module
  ModuleName name = "DucksTimeToReachBallProvider";
  DucksTimeToReachBallProvider(const ModuleManagerInterface& manager);
  void cycle();

private:
  /// the time added as a penalty if the robot is fallen
  const Parameter<float> fallenPenalty_;
  /// the time substract if we are striker (hysteresis, for robust decision)
  const Parameter<float> strikerBonus_;
  /// the time added if we didn't see the ball ourselfs
  const Parameter<float> ballNotSeenPenalty_;

  const Dependency<BodyPose> bodyPose_;
  const Dependency<GameControllerState> gameControllerState_;
  const Dependency<RobotPosition> robotPosition_;
  const Dependency<DucksStrikerAction> ducksStrikerAction_;
  const Dependency<TeamBallModel> teamBallModel_;
  const Dependency<WalkingEngineWalkOutput> walkingEngineWalkOutput_;
  Production<TimeToReachBall> timeToReachBall_;
};
