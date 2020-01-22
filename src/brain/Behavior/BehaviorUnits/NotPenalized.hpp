#pragma once
#include "Behavior/Units.hpp"

ActionCommand notPenalized(const DataSet& d)
{
  // Special conditional checks introduced to enable penalty behavior.
  const bool isPenaltyWait = d.gameControllerState.gamePhase != GamePhase::PENALTYSHOOT ||
                             d.penaltyKeeperAction.type == PenaltyKeeperAction::Type::WAIT;

  if (d.gameControllerState.gameState == GameState::INITIAL)
  {
    return initial(d)
        .combineLeftLED(ActionCommand::EyeLED::rainbow())
        .combineRightLED(ActionCommand::EyeLED::rainbow());
  }
  else if (d.gameControllerState.gameState == GameState::FINISHED)
  {
    return finished(d);
  }
  else if (d.bodyPose.fallen && isPenaltyWait)
  {
    return standUp(d);
  }
  else
  {
    const float ballAge = d.cycleInfo.getTimeDiff(d.ballState.timeWhenLastSeen);
    ActionCommand::EyeLED ballLED = ActionCommand::EyeLED::off();
    if (ballAge < 0.3)
    {
      ballLED = ActionCommand::EyeLED::red();
    }
    else if (ballAge < 1.f)
    {
      ballLED = ActionCommand::EyeLED::yellow();
    }
    else if (ballAge < 2.5f)
    {
      ballLED = ActionCommand::EyeLED::lightblue();
    }
    else if (ballAge < 5.f)
    {
      ballLED = ActionCommand::EyeLED::blue();
    }
    if (d.gameControllerState.gameState == GameState::READY)
    {
      return ready(d).combineLeftLED(ballLED);
    }
    else if (d.gameControllerState.gameState == GameState::SET)
    {
      return set(d).combineLeftLED(ballLED);
    }
    else if (d.gameControllerState.gameState == GameState::PLAYING)
    {
      return playing(d).combineLeftLED(ballLED);
    }
    return ActionCommand::stand();
  }
}
