#include <cmath>

#include "Tools/Chronometer.hpp"
#include "Tools/Math/Angle.hpp"
#include "Tools/Math/Hysteresis.hpp"
#include "Tools/Math/Range.hpp"
#include "Tools/PermissionManagement.hpp"

#include "DucksKeeperActionProvider.hpp"

DucksKeeperActionProvider::DucksKeeperActionProvider(const ModuleManagerInterface& manager)
  : Module(manager)
  , cycleInfo_(*this)
  , ballState_(*this)
  , fieldDimensions_(*this)
  , gameControllerState_(*this)
  , robotPosition_(*this)
  , teamBallModel_(*this)
  , teamPlayers_(*this)
  , worldState_(*this)
  , keeperAction_(*this)
  , keeperPosition_(Vector2f((-fieldDimensions_->fieldLength * 0.5f) +
                                 fieldDimensions_->fieldPenaltyAreaLength * 0.5f,
                             0.f))
{
}

void DucksKeeperActionProvider::cycle()
{
  Chronometer time(debug(), mount_ + ".cycle_time");
  if (gameControllerState_->gameState != GameState::PLAYING)
  {
    return;
  }
}